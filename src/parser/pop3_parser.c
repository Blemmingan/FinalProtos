#include "pop3_parser.h"

static void process_command(parser_event *event, const uint8_t c) {
    if (event->command_length < MAX_COMMAND_LEN) {
        event->command[event->command_length++] = c;
    }
}

static void process_argument(parser_event *event, const uint8_t c) {
    if (event->args_length < MAX_ARG_LEN) {
        event->args[event->args_length++] = c;
    }
}

static void finalize(parser_event *event, const uint8_t c) {
    event->done = true;
}