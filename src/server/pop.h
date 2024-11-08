#ifndef poppuntoh
#define poppuntoh

#define MAX_BUFFER_SIZE 1024

void pop3_passive_accept(struct selector_key *key);
void echo_handle_read(struct selector_key *key);
void echo_handle_close(struct selector_key *key) ;
#endif