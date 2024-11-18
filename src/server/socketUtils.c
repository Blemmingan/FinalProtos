#include "socketUtils.h"

int create_server_socket(const char *addr, int port){

    //  sockaddr_in has both port number and IP address.
    struct sockaddr_in address;

    //  Does pretty much the same as calloc, except this memory has already been alloc'd.
    memset(&address, 0, sizeof(address));

     //  You are an IPV4 socket.
    address.sin_family = AF_INET;

    //  Converts port number to the correct format to use with sockets.
    //  Socket will listen on port number specified.
    address.sin_port = htons(port);

    if (inet_pton(AF_INET, addr, &address.sin_addr) < 0){
        fprintf(stderr, "Invalid address\n");
        return -1;
    }

    // Creates the socket. Socket will use IPV4, a stream-based protocol, and TCP.
    // Socket file descriptor will be saved in the variable.
    const int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == -1){
        return -1;
    }

    fprintf(stdout, "Listening on IP: %s and TCP port: %d\n", addr, port);
    
    // SOL_SOCKET means this is a socket-level option.
    // REUSEADDR means it can reuse local addresses.
    // The {1} is a temporary value to set the various options to 1.
    // The sizeof is an indicator of the size of the value we are setting.
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if(bind(serverSocket, (struct sockaddr *) &address, sizeof(address)) < 0){
        fprintf(stderr, "Unable to bind passive socket.\n");
        return -1;
    }

    if (listen(serverSocket, MAX_PENDING_CONNECTIONS) < 0) {
        fprintf(stderr, "Passive socket unable to listen.\n");
        return -1;
    }
    
    printf("Socket is bound and listening\n");
    return serverSocket;
}