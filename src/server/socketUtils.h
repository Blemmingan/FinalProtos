#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_PENDING_CONNECTIONS 20

int create_server_socket(const char *addr, int port);

#endif // SOCKET_UTILS_H
