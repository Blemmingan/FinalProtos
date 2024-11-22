#ifndef poppuntoh
#define poppuntoh

#define MAX_BUFFER_SIZE 4000
#define GREETING "+OK READY\r\n"
#define BASE_DIR "/home/usuarios/"

enum pop_commands{
    USER,
    PASS,
    STAT,
    LIST,
    RETR,
    DELE, 
    NOOP,
    QUIT,
};

//  Handles accepting connections to the passive socket.
void pop3_passive_accept(struct selector_key *key);

//  Handles read operations for the echo server.
void echo_handle_read(struct selector_key *key);

//  Handles write operations for the echo server.
void echo_handle_write(struct selector_key * key);

//  Handles client closing the connection.
void echo_handle_close(struct selector_key *key) ;

int parse_command(int fd, buffer * buff );

void process_client_request(int client_fd);

int validate_user_directory(buffer* input_buffer);
#endif