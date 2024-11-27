#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 10080
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while (1) {
        memset(buffer, 0, sizeof(buffer));  // Clear the buffer
        bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("recv failed");
            }
            break;
        }
        
        // Null-terminate the received data
        buffer[bytes_received] = '\0';

        // Debugging: Print the raw received command
        //printf("Raw received command: '%s'\n", buffer);

        // Remove leading '\r' and '\n' characters from the string
        char *command = buffer;
        while (*command == '\r' || *command == '\n') {
            command++;
        }

        // Remove trailing '\r' and '\n' characters from the string
        size_t len = strlen(command);
        while (len > 0 && (command[len - 1] == '\r' || command[len - 1] == '\n')) {
            command[len - 1] = '\0';
            len--;
        }

        // Debugging: Print the cleaned-up command
        //printf("Cleaned-up command: '%s'\n", command);

        // Skip empty commands
        if (strlen(command) == 0) {
            continue;  // Skip the iteration if the command is empty
        }

        // Process the command
        if (strcmp(command, "HELLO") == 0) {
            send(client_socket, "Hello, Client!\n", 15, 0);
        } else if (strcmp(command, "GOODBYE") == 0) {
            send(client_socket, "Goodbye, Client!\n", 17, 0);
        } else {
            send(client_socket, "Unknown command\n", 17, 0);
        }
    }

    // Close the client socket
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create the server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR to allow the server to reuse the port immediately
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        perror("setsockopt failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 3) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept incoming client connections
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) != -1) {
        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

        // Handle the client communication
        handle_client(client_socket);
    }

    if (client_socket == -1) {
        perror("Accept failed");
    }

    // Close the server socket
    close(server_socket);
    return 0;
}

