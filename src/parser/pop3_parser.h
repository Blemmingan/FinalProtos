#include <stdbool.h>

#include "parser.h"
// Número máximo de estados en el parser.
#define MAX_STATES 5

#define N(x) (sizeof(x) / sizeof((x)[0]))

enum pop3_states{
   INTITAL_STATE,
   PROCESSING_COMMAND,
   PROCESSING_ARGS,
   TERMINATION_STATE,
   ERROR_STATE
};

typedef struct {
    const struct parser_state_transition *transitions;  // Transiciones de este estado
    size_t transition_count;                            // Número de transiciones para este estado
} state_definition;

static void process_command(parser_event *event, const uint8_t c);
static void process_argument(parser_event *event, const uint8_t c);
static void finalize(parser_event *event, const uint8_t c);

static const struct parser_state_transition INITIAL_TRANSITION[] = {
    { .when = ANY, .dest = PROCESSING_COMMAND, .act1 = process_command},
    { .when = ' ', .dest = ERROR_STATE, .act1 = NULL},
    { .when = '\r', .dest = ERROR_STATE, .act1 = NULL}
};

static const struct parser_state_transition PROCESSING_COMMAND_TRANSITION[] = {
    { .when = ANY, .dest = PROCESSING_COMMAND, .act1 = process_command},
    { .when = ' ', .dest = PROCESSING_ARGS, .act1 = process_argument},
    { .when = '\r', .dest = ERROR_STATE, .act1 = NULL}
};

static const struct parser_state_transition PROCESSING_ARGS_TRANSITION[] = {
    { .when = ANY, .dest = PROCESSING_ARGS, .act1 = process_argument},
    { .when = '\r', .dest = TERMINATION_STATE, .act1 = NULL}
};

static const struct parser_state_transition TERMINATION_TRANSITION[] = {
    { .when = ANY, .dest = ERROR_STATE, .act1 = NULL},
    { .when = '\n', .dest = INTITAL_STATE, .act1 = finalize}
};

static const struct parser_state_transition ERROR_TRANSITION[] = {
    { .when = ANY, .dest = ERROR_STATE, .act1 = NULL},
    { .when = '\r', .dest = TERMINATION_STATE, .act1 = NULL}
};

static const state_definition state_definitions[MAX_STATES] = {
    [INTITAL_STATE] = { INITIAL_TRANSITION, N(INITIAL_TRANSITION) },
    [PROCESSING_ARGS] = { PROCESSING_ARGS_TRANSITION, N(PROCESSING_ARGS_TRANSITION) },
    [PROCESSING_COMMAND] = { PROCESSING_ARGS_TRANSITION, N(PROCESSING_ARGS_TRANSITION) },
    [TERMINATION_STATE] = { TERMINATION_TRANSITION, N(TERMINATION_TRANSITION) },
    [ERROR_STATE] = { ERROR_TRANSITION, N(ERROR_TRANSITION) }
};

const struct parser_definition pop3_parser = {
    .start_state = INTITAL_STATE,
    .states = state_definitions,
    .states_count = MAX_STATES,
};