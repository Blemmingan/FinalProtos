#include<stdio.h>
#include <stdlib.h>  // malloc
#include <string.h>  // memset
#include <assert.h>  // assert
#include <errno.h>
#include <time.h>
#include <unistd.h>  // close
#include <pthread.h>
#include <sys/stat.h>
#include <arpa/inet.h>


#include "buffer.h"
#include "selector.h"
#include "stm.h"
#include"netutils.h"
#include "pop.h"
#include "mainServer.h"
#include "args.h"
#define BUFFER_SIZE 512  

char user_path[BUFFER_SIZE];

typedef struct pop3{
    struct state_machine stateMachine;
    //  mas cosas...
}pop3;


// Echo handler for reading data
void echo_handle_read(struct selector_key *key) {

    // Read data from the client
    ssize_t nbytes;
    ssize_t bytes_read = recv(key->fd, buffer_read_ptr(key->data, &nbytes), MAX_BUFFER_SIZE, 0);
    if (bytes_read == -1) {
        perror("Error reading from client");
        close(key->fd);
        return;
    }
    nbytes += bytes_read;
    buffer_write_adv(key->data, nbytes);

    if(bytes_read != 0){
        parse_command(key->fd, key->data);
    }
    else if (bytes_read == 0) {
        // Client disconnected (clean close)
        printf("Client disconnected, preparing to close connection...\n");
        buffer_reset(key->data);
        // This ensures that the cleanup will happen later, once the event loop is done processing
        // The cleanup will be done in the close handler when the socket is unregistered
        selector_set_interest(key->s, key->fd, OP_NOOP);  // Temporarily remove it from the selector
        selector_unregister_fd(key->s, key->fd);
        return;
    }


    if (buffer_can_read(key->data)) {
    // Only set OP_WRITE if it's not already set
        selector_set_interest(key->s, key->fd, OP_WRITE);
    } else {
    // If nothing to read, reset the interest or set it back to OP_READ if needed
        selector_set_interest(key->s, key->fd, OP_READ);
}

    
}

void echo_handle_write(struct selector_key * key){
    // Echo the data back to the client
    size_t nbytes;
    ssize_t bytes_written = send(key->fd, buffer_read_ptr(key->data, &nbytes), buffer_position(key->data), 0);
    nbytes += bytes_written;
    buffer_read_adv(key->data, bytes_written);
    if (bytes_written == -1) {
        perror("Error writing to client");
        close(key->fd);
        return;
    }
    if (!buffer_can_read(key->data)) {
        // If no more data to read, stop interest in writing
        selector_set_interest(key->s, key->fd, OP_READ);
        return;
    }
}

// Echo handler for closing the connection properly
void echo_handle_close(struct selector_key *key) {

    printf("Closing connection: fd = %d\n", key->fd);

/// Unregister from the selector.
    //selector_unregister_fd(key->s, key->fd);
    // Decrease the number of current connections
    current_connections--;
    // Close the socket
    close(key->fd);
}

// Accept handler when a new client connects
void pop3_passive_accept(struct selector_key *key) {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(key->fd, (struct sockaddr*)&client_addr, &client_addr_len);



    buffer buffer;
    buffer.data = NULL;
    buffer.limit = NULL;
    buffer.read = NULL;
    buffer.write = NULL;
    uint8_t buf[MAX_BUFFER_SIZE];
    buffer_init(&buffer, MAX_BUFFER_SIZE, buf);

    if (client_fd == -1) {
        perror("Accept failed");
        return;
    }
    else{
        total_connections++;  // Increase the total number of connections
        current_connections++;  // Increase the number of current connections
    }

    printf("Accepted a connection from client\n");

    // Set the handler for the accepted client connection
    const struct fd_handler echo_handler = {
        .handle_read = echo_handle_read,
        .handle_write = echo_handle_write,  // We'll handle writing in the read handler
        .handle_close = echo_handle_close
    };

    // Register the client socket with the selector for OP_READ (only read initially)
    selector_status status = selector_register(key->s, client_fd, &echo_handler, OP_READ, &buffer);

    if (status != SELECTOR_SUCCESS) {
        fprintf(stderr, "Failed to register client fd with selector: %s\n", selector_error(status));
        close(client_fd);
    } else {
        printf("Client registered for reading\n");
    }
    version(client_fd);
} 

int parse_command(int fd, buffer * buff ){

    if (strncmp(buff->data, "USER ", 5) == 0) {
        printf("Received a USER command");
        return USER;
        
    }
    else{
        //usage(fd, buff);
    }
    /* mas else con los demas comandos 
    else if (strncmp(, ,) == 0) {    
    } 
    */
    return 0;
}

void process_client_request(int client_fd) {
    char client_buffer[BUFFER_SIZE];
    int bytes_received;
    buffer * client_buffer_struct = malloc(sizeof(buffer));
    buffer_init(client_buffer_struct, BUFFER_SIZE, client_buffer);
    char * message="+OK POP3 server ready\r\n" ;
    send(client_fd, message, strlen( message), 0);
    while (1) {
        char* response;
        bytes_received = recv(client_fd, client_buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("recv error");
            }
            break;
        }
        buffer_compact(client_buffer_struct); 
        buffer_write_adv(client_buffer_struct, bytes_received - 1); 
        buffer_write(client_buffer_struct, '\0');
        char* command = buffer_read_ptr(client_buffer_struct, &(size_t) {4});
        buffer_read_adv(client_buffer_struct, (size_t) 4);
        switch (parse_command(client_fd, command)) {
            case USER:
                if (buffer_read(client_buffer_struct) == '\0') {
                    response = "-ERR Missing username. Please provide a valid username.\r\n";
                    send(client_fd, response, strlen(response), 0);
                } else {
                    if (validate_user_directory(client_buffer_struct)) {
                        response = "-ERR User not found. Please check the username and try again.\r\n";
                        send(client_fd, response, strlen(response), 0);
                    } else {
                        response = "+OK User accepted. Password is required.\r\n";
                        send(client_fd, response, strlen(response), 0);
                        /* hacer load_user_data 
                        if (load_user_data()) {
                            response = "-ERR Error loading user data. Please try again later.\r\n";
                            send(client_fd, response, strlen(response), 0);
                        }
                        */
                    }
                }
                break;

            // otros comandos que hay que agregar en pop.h y ejecutarlos aca 
        }
    }
}


int validate_user_directory(buffer* input_buffer) {
    char* username = buffer_read_ptr(input_buffer, &(size_t) {input_buffer->write - input_buffer->read});
    buffer_read_adv(input_buffer, input_buffer->write - input_buffer->read);
    sprintf(user_path, "%s%s", BASE_DIR, username);
    struct stat file_status;
    if (stat(user_path, &file_status) == 0) {
        if (S_ISDIR(file_status.st_mode)) {
            printf("El directorio '%s' fue  encontrado.\n", user_path);
        } else {
            printf("El archivo '%s' fue encontrado, pero no es un directorio.\n", user_path);
            return 2;  
        }
    } else {
        perror("No se pudo obtener información del directorio");
        return 1;  
    }
    return 0;  
}
