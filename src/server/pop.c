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

typedef struct pop3{
    struct state_machine stateMachine;
}pop3;


/** Intenta aceptar la nueva conexión entrante*/
void
pop3_passive_accept(struct selector_key *key) {
    struct sockaddr_storage       client_addr;
    socklen_t                     client_addr_len = sizeof(client_addr);
    struct pop3                *state           = NULL;

    const int client = accept(key->fd, (struct sockaddr*) &client_addr,
                                                          &client_addr_len);
    if(client == -1) {
        printf("Accept fail\n");
        exit(1);
    }
}