#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 10080
#define BUFFER_SIZE 1024

// Function to trim trailing and leading whitespaces or newline characters
void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';  // Remove trailing newline
    }
    if (len > 0 && str[len - 1] == '\r') {
        str[len - 1] = '\0';  // Remove trailing carriage return
    }
}

// Function to handle the commands
void process_command(int client_socket, char *command) {
    trim_newline(command);  // Remove any extra newline or carriage return characters

    if (strcmp(command, "HELLO") == 0) {
        const char *response = "Hello, Client!\n";
        send(client_socket, response, strlen(response), 0);
    } else if (strcmp(command, "GOODBYE") == 0) {
        const char *response = "Goodbye, Client!\n";
        send(client_socket, response, strlen(response), 0);
    } else {
        const char *response = "Unknown command\n";
        send(client_socket, response, strlen(response), 0);
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int bytes_read;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

        while (1) {
            memset(buffer, 0, sizeof(buffer));
            bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read <= 0) {
                if (bytes_read == 0) {
                    printf("Client disconnected\n");
                } else {
                    perror("Recv failed");
                }
                break;
            }

            printf("Received command: '%s'\n", buffer);
            process_command(client_socket, buffer);
        }

        close(client_socket);
    }

    close(server_fd);
    return 0;
}
