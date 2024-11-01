/**
 * main.c - servidor proxy socks concurrente
 *
 * Interpreta los argumentos de línea de comandos, y monta un socket
 * pasivo.
 *
 * Todas las conexiones entrantes se manejarán en éste hilo.
 *
 * Se descargará en otro hilos las operaciones bloqueantes (resolución de
 * DNS utilizando getaddrinfo), pero toda esa complejidad está oculta en
 * el selector.
 */
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

#include "socks5.h"
#include "selector.h"
#include "socks5nio.h"

//	Done se usa para terminar la ejecucion.
static bool done = false;

//	Manejo de señales.
static void
sigterm_handler(const int signal) {
    printf("signal %d, cleaning up and exiting\n",signal);
    done = true;
}

int
main(const int argc, const char **argv) {
    unsigned port = 1080; //	Puerto que se usa por defecto

    if(argc == 1) {
        // utilizamos el default
    } else if(argc == 2) {
	//	Si me especifican un puerto, usar ese.
        char *end     = 0;
        const long sl = strtol(argv[1], &end, 10);

	//	Chequeo del numero pasado como puerto
        if (end == argv[1]|| '\0' != *end 
           || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
           || sl < 0 || sl > USHRT_MAX) {
            fprintf(stderr, "port should be an integer: %s\n", argv[1]);
            return 1;
        }
        port = sl;
    } else {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    // no tenemos nada que leer de stdin por lo que cierro stdin, soy un servidor
    close(0);

    const char       *err_msg = NULL;
    selector_status   ss      = SELECTOR_SUCCESS;
    fd_selector selector      = NULL;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET; //	Uso el protocolo IPV4.
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);	//	Guardo el numero de puerto en formato servidor

    const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	//	CREO EL SOCKET
    if(server < 0) {
        err_msg = "unable to create socket";
        goto finally;
    }

    fprintf(stdout, "Listening on TCP port %d\n", port);

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
    // SO_REUSEADDR:
    //Indicates that the rules used in validating addresses supplied in a bind(2) call should allow 
    // reuse of local addresses. For AF_INET sockets this means that a socket may bind,
    // except when there is an active listening socket bound to the address. 
    //When the listening socket is bound to INADDR_ANY with a specific port then it is not possible 
    //to bind to this port for any local address. Argument is an integer boolean flag	

   //SOL_SOCKET hace que aplique REUSEADDR a todos los sockets.

    //	Ahora hago BIND y LISTEN.

    if(bind(server, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        err_msg = "unable to bind socket";
        goto finally;
    }

    if (listen(server, 20) < 0) {
        err_msg = "unable to listen";
        goto finally;
    }

    // registrar sigterm es útil para terminar el programa normalmente.
    // esto ayuda mucho en herramientas como valgrind.
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);

    // Seteo los flags no bloqueantes.
    if(selector_fd_set_nio(server) == -1) {
        err_msg = "getting server socket flags";
        goto finally;
    }
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
        err_msg = "initializing selector";
        goto finally;
    }

    // Aca verdaderamente se crea el selector con 1024 elementos. 
    selector = selector_new(1024);
    if(selector == NULL) {
        err_msg = "unable to create selector";
        goto finally;
    }

    // Handler para el socket pasivo, solo acepta la conexion.
    const struct fd_handler socksv5 = {
        .handle_read       = socksv5_passive_accept,
        .handle_write      = NULL,
        .handle_close      = NULL, // nada que liberar
    };

   // Registra en la tabla de FDs.
    ss = selector_register(selector, server, &socksv5,
                                              OP_READ, NULL);
    if(ss != SELECTOR_SUCCESS) {
        err_msg = "registering fd";
        goto finally;
    }


    // ESTO VA A SEGUIR HASTA QUE TERMINE EL PROGRAMA. Hace select!!!!!
    // Si ss no esta en SELECTOR_SUCCESS es porque el servidor esta sirviendo otros sockets.
    for(;!done;) {
        err_msg = NULL;
        ss = selector_select(selector);
        if(ss != SELECTOR_SUCCESS) {
            err_msg = "serving";
            goto finally;
        }
    }
    if(err_msg == NULL) {
        err_msg = "closing";
    }

    int ret = 0;
finally:
    // Si hubo error...
    if(ss != SELECTOR_SUCCESS) {
        fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "": err_msg,
                                  ss == SELECTOR_IO
                                      ? strerror(errno)
                                      : selector_error(ss));
        ret = 2;
    } else if(err_msg) {
        perror(err_msg);
        ret = 1;
    }
    if(selector != NULL) {
        selector_destroy(selector);
    }
    selector_close();

    socksv5_pool_destroy();

    if(server >= 0) {
        close(server);
    }
    return ret;
}
