#include <stdbool.h>

#include "parser.h"
// Número máximo de estados en el parser.
#define MAX_STATES 8

#define N(x) (sizeof(x) / sizeof((x)[0]))

enum pop3_states{
   INTITAL_STATE,
   FIRST_CHAR_STATE,
   SECOND_CHAR_STATE,
   THIRD_CHAR_STATE,
   FOURTH_CHAR_STATE,
   ARGS_STATE,
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

static const struct parser_state_transition INIT_TRANSITION[] = {
    { .when = ANY, .dest = FIRST_CHAR_STATE, .act1 = process_command},
    { .when = ' ', .dest = ERROR_STATE, .act1 = NULL},
    { .when = '\r', .dest = ERROR_STATE, .act1 = NULL}
};

static const struct parser_state_transition FIRST_CHAR_TRANSITION[] = {
    { .when = ANY, .dest = SECOND_CHAR_STATE, .act1 = process_command},
    { .when = ' ', .dest = ERROR_STATE, .act1 = NULL},
    { .when = '\r', .dest = ERROR_STATE, .act1 = NULL}
};

static const struct parser_state_transition SECOND_CHAR_TRANSITION[] = {
    { .when = ANY, .dest = THIRD_CHAR_STATE, .act1 = process_command},
    { .when = ' ', .dest = ERROR_STATE, .act1 = NULL},
    { .when = '\r', .dest = ERROR_STATE, .act1 = NULL}
};

static const struct parser_state_transition THIRD_CHAR_TRANSITION[] = {
    { .when = ANY, .dest = FOURTH_CHAR_STATE, .act1 = process_command},
    { .when = ' ', .dest = ARGS_STATE, .act1 = process_argument},
    { .when = '\r', .dest = TERMINATION_STATE, .act1 = NULL}
};

static const struct parser_state_transition FOURTH_CHAR_TRANSITION[] = {
    { .when = ANY, .dest = ERROR_STATE, .act1 = NULL},
    { .when = ' ', .dest = ARGS_STATE, .act1 = process_argument},
    { .when = '\r', .dest = TERMINATION_STATE, .act1 = NULL}
};

static const struct parser_state_transition ARGS_TRANSITION[] = {
    { .when = ANY, .dest = ARGS_STATE, .act1 = process_argument},
    { .when = '\r', .dest = TERMINATION_STATE, .act1 = NULL}
};

static const struct parser_state_transition TERMINATION_TRANSITION[] = {
    { .when = ANY, .dest = ERROR_STATE, .act1 = NULL},
    { .when = '\r', .dest = TERMINATION_STATE, .act1 = NULL},
    { .when = '\n', .dest = INTITAL_STATE, .act1 = finalize}
};

static const struct parser_state_transition ERROR_TRANSITION[] = {
    { .when = ANY, .dest = ERROR_STATE, .act1 = NULL},
    { .when = '\r', .dest = TERMINATION_STATE, .act1 = NULL}
};

static const state_definition state_definitions[MAX_STATES] = {
    [INTITAL_STATE] = { INIT_TRANSITION, N(INIT_TRANSITION) },
    [FIRST_CHAR_STATE] = { FIRST_CHAR_TRANSITION, N(FIRST_CHAR_TRANSITION) },
    [SECOND_CHAR_STATE] = { SECOND_CHAR_TRANSITION, N(SECOND_CHAR_TRANSITION) },
    [THIRD_CHAR_STATE] = { THIRD_CHAR_TRANSITION, N(THIRD_CHAR_TRANSITION) },
    [FOURTH_CHAR_STATE] = { FOURTH_CHAR_TRANSITION, N(FOURTH_CHAR_TRANSITION) },
    [ARGS_STATE] = { ARGS_TRANSITION, N(ARGS_TRANSITION) },
    [TERMINATION_STATE] = { TERMINATION_TRANSITION, N(TERMINATION_TRANSITION) },
    [ERROR_STATE] = { ERROR_TRANSITION, N(ERROR_TRANSITION) }
};

const struct parser_definition pop3_parser = {
    .start_state = INTITAL_STATE,
    .states = state_definitions,
    .states_count = MAX_STATES,
};