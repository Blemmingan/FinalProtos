#ifndef SERVER_H
#define SERVER_H

// Declare the global connection counters as external
extern int total_connections;
extern int current_connections;

static void print_connection_stats();

#endif // SERVER_H
