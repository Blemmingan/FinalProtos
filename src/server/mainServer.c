#include "buffer.h"
#include "netutils.h"
#include "parser_utils.h"
#include "parser.h"
#include "selector.h"
#include "stm.h"
#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netinet/in.h>
#include <netinet/tcp.h>



#define DEFAULT_PORT 10080
#define MAX_PENDING_CONNECTIONS 20
int main(int argc, char ** argv){
    
    //  PORT NUMBER MANAGEMENT

    int port = DEFAULT_PORT;
    if (argc == 1){
        //  Use default port
    }
    else if (argc == 2){
        //	Use specified port.
        char *end     = 0;
        const long sl = strtol(argv[1], &end, 10);

	//	Checks whether the port number is both valid and a number.  
        if (end == argv[1]|| '\0' != *end 
           || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
           || sl < 0 || sl > USHRT_MAX) {
            fprintf(stderr, "port should be an integer: %s\n", argv[1]);
            return 1;
        }
        //  New port used will be the port given by command line arguments.
        port = sl;
    }
    else{
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        return 1;
    }


    //  I close stdin because I am a server, I do not read from stdin.
    close(0);

    const char * errorMessage = NULL;
    //  Selector must begin in SUCCESS by default.
    selector_status selectorStatus = SELECTOR_SUCCESS;
    fd_selector selector = NULL;

    //  sockaddr_in has both port number and IP address.
    struct sockaddr_in address;
    //  Does pretty much the same as calloc, except this memory has already been alloc'd.
    memset(&address, 0, sizeof(address));

    //  You are an IPV4 socket.
    address.sin_family = AF_INET;
    //  INADDR_ANY is actually 0.0.0.0, port will listen.
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    //  Converts port number to the correct format to use with sockets.
    //  Socket will listen on port number specified.
    address.sin_port = htonl(port);

    //  Creates the socket. Socket will use IPV4, a stream-based protocol, and TCP.
    //  Socket file descriptor will be saved in the variable.
    const int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(serverSocket == -1){
        errorMessage = "Socket creation failed.\n";
        goto finally;
    }

    fprintf(stdout, "Listening on TCP port %d\n", port);

    //  SOL_SOCKET means this is a socket-level option.
    //  REUSEADDR means it can reuse local addresses.
    //  The {1} is a temporary value to set the various options to 1.
    //  The sizeof is an indicator of the size of the value we are setting.
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if(bind(serverSocket, (struct sockaddr *) &address, sizeof(address)) < 0){
        errorMessage = "Unable to bind passive socket.\n";
        goto finally;
    }

    if (listen(serverSocket, MAX_PENDING_CONNECTIONS) < 0) {
        errorMessage = "Passive socket is unable to listen\n";
        goto finally;
    }

    printf("Socket is bound and listening\n");
    int ret = 0;
finally: 
    //  If there was an error...
    if(selectorStatus != SELECTOR_SUCCESS) {
        fprintf(stderr, "%s: %s\n", (errorMessage == NULL) ? "": errorMessage,
                                  selectorStatus == SELECTOR_IO
                                      ? strerror(errno)
                                      : selector_error(selectorStatus));
        ret = 2;
    } else if(errorMessage) {
        perror(errorMessage);
        ret = 1;
    }
    if(selector != NULL) {
        selector_destroy(selector);
    }
    selector_close();

    if(serverSocket >= 0) {
        close(serverSocket);
    }
    return ret;


}