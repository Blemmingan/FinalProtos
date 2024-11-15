#ifndef poppuntoh
#define poppuntoh

#define MAX_BUFFER_SIZE 4000

//  Handles accepting connections to the passive socket.
void pop3_passive_accept(struct selector_key *key);

//  Handles read operations for the echo server.
void echo_handle_read(struct selector_key *key);

//  Handles write operations for the echo server.
void echo_handle_write(struct selector_key * key);

//  Handles client closing the connection.
void echo_handle_close(struct selector_key *key) ;
#endif