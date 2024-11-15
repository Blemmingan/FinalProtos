#include<stdio.h>
#include <stdlib.h>  // malloc
#include <string.h>  // memset
#include <assert.h>  // assert
#include <errno.h>
#include <time.h>
#include <unistd.h>  // close
#include <pthread.h>

#include <arpa/inet.h>


#include "buffer.h"
#include "selector.h"
#include "stm.h"
#include"netutils.h"
#include "pop.h"

typedef struct pop3{
    struct state_machine stateMachine;
    //  mas cosas...
}pop3;


// Echo handler for reading data
void echo_handle_read(struct selector_key *key) {

    // Read data from the client
    ssize_t nbytes;
    ssize_t bytes_read = recv(key->fd, buffer_read_ptr(key->data, &nbytes), MAX_BUFFER_SIZE, 0);
    nbytes += bytes_read;
    buffer_write_adv(key->data, nbytes);
    if (bytes_read == -1) {
        perror("Error reading from client");
        close(key->fd);
        return;
    }

    if (bytes_read == 0) {
        // Client disconnected (clean close)
        printf("Client disconnected, preparing to close connection...\n");
        buffer_reset(key->data);
        // This ensures that the cleanup will happen later, once the event loop is done processing
        // The cleanup will be done in the close handler when the socket is unregistered
        selector_set_interest(key->s, key->fd, OP_NOOP);  // Temporarily remove it from the selector
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
    }
}

// Echo handler for closing the connection properly
void echo_handle_close(struct selector_key *key) {

    printf("Closing connection: fd = %d\n", key->fd);

/// Unregister from the selector.
    selector_unregister_fd(key->s, key->fd);
    
    // Close the socket
    close(key->fd);
}

// Accept handler when a new client connects
void pop3_passive_accept(struct selector_key *key) {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(key->fd, (struct sockaddr*)&client_addr, &client_addr_len);

    //char buffer[MAX_BUFFER_SIZE]; debe ser un tipo buffer
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
}