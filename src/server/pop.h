#ifndef poppuntoh
#define poppuntoh

#define MAX_BUFFER_SIZE 4000
#define GREETING "+OK READY\r\n"

// AUTHORIZATION
#define USER           "USER"
#define PASS           "PASS"

#define ERR_COMMAND    "-ERR unknown command\r\n"

#define OK_USER        "+OK USER\r\n"
#define OK_PASS        "+OK PASS\r\n"
#define ERR_USER       "-ERR USER\r\n"
#define ERR_PASS_VALID "-ERR PASS: invalid password\r\n"


//  Handles accepting connections to the passive socket.
void pop3_passive_accept(struct selector_key *key);

//  Handles read operations for the echo server.
void echo_handle_read(struct selector_key *key);

//  Handles write operations for the echo server.
void echo_handle_write(struct selector_key * key);

//  Handles client closing the connection.
void echo_handle_close(struct selector_key *key) ;
#endif