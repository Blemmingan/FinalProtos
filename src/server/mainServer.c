#include "buffer.h"
#include "netutils.h"
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
#include "pop.h"
#include "mainServer.h"
#include "args.h"
#include "socketUtils.h"

// Contadores globales de conexiones.
int total_connections = 0;
int current_connections = 0;

struct pop3args * args;

static bool done = false;

//	Manejo de señales.
static void
sigterm_handler(const int signal) {
    printf("signal %d, cleaning up and exiting\n",signal);
    done = true;
}

static void print_connection_stats() {
    printf("\nTotal connections: %d, Current connections: %d\n", total_connections, current_connections);
}

int main(int argc, char ** argv){
    
    // args tiene: pop3_addr, pop3_port, mng_addr, mng_port, users[MAX_USERS], directory[MAX_DIRECTORY_LENGTH];
    args = (struct pop3args *) malloc (sizeof(struct pop3args)); 

    parse_args(argc, argv, args);

    printf("Starting server...\n");
    //  I close stdin because I am a server, I do not read from stdin.
    close(0);

    const char * errorMessage = NULL;

    //  Selector must begin in SUCCESS by default.
    selector_status selectorStatus = SELECTOR_SUCCESS;
    fd_selector selector = NULL;

    int serverSocket = create_server_socket(args->pop3_addr, args->pop3_port);

    if(serverSocket == -1){
        errorMessage = "unable to create socket";
        goto finally;
    }

    // Acá iria el create_manager_socket

    // registrar sigterm es útil para terminar el programa normalmente.
    // esto ayuda mucho en herramientas como valgrind.
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);


    // Seteo los flags no bloqueantes.
    if(selector_fd_set_nio(serverSocket) == -1) {
        errorMessage = "getting server socket flags";
        goto finally;
    }
    printf("Nonblock selector flags set\n");
    const struct selector_init conf = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec  = 10,
            .tv_nsec = 0,
        },
    };

    // Inicializo el selector con el struct que puse arriba. 
    // En particular, le paso la señal que quiero que intercepte
    // y los timeouts para el select.
    if(0 != selector_init(&conf)) {
        errorMessage = "initializing selector";
        goto finally;
    }
    printf("Selector initialized\n");

    // Aca verdaderamente se crea el selector con 1024 elementos. 
    selector = selector_new(1024);
    if(selector == NULL) {
        errorMessage = "unable to create selector";
        goto finally;
    }
    printf("Selector created\n");

    // Handler para el socket pasivo, solo acepta la conexion.
    const struct fd_handler pop = {
        .handle_read       = pop3_passive_accept,
        .handle_write      = NULL,
        .handle_close      = NULL, // nada que liberar
    };


   // Registra en la tabla de FDs.
    selectorStatus = selector_register(selector, serverSocket, &pop,
                                              OP_READ, NULL);
    if(selectorStatus != SELECTOR_SUCCESS) {
        errorMessage = "registering fd";
        goto finally;
    }
    printf("File descriptor for passive socket registered\n");

    // ESTO VA A SEGUIR HASTA QUE TERMINE EL PROGRAMA. Hace select!!!!!
    // Si ss no esta en SELECTOR_SUCCESS es porque el servidor esta sirviendo otros sockets.
    for(;!done;) {
        errorMessage = "You killed the program with CTRL-C\n";
        selectorStatus = selector_select(selector);
    
        if(selectorStatus != SELECTOR_SUCCESS) {
            errorMessage = "serving";
            goto finally;
        }
        //printf("WAITING FOR ACCEPT");
        print_connection_stats();
    }
    if(errorMessage == NULL) {
        errorMessage = "closing";
    }

    int ret = 0;

finally: 
    printf("In FINALLY\n");
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