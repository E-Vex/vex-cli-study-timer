#ifndef CLI_H
#define CLI_H

#include <stdio.h>

#include "../timer/timer.h"

typedef enum
{
    CLI_RESULT_ERROR = 0,
    CLI_RESULT_OK = 1,
    CLI_RESULT_HELP = 2,
    CLI_RESULT_VERSION = 3
} cli_result_t;

void print_usage(FILE *stream);
void print_version(void);
cli_result_t parse_timer_args(int argc, char *argv[], timer_config_t *timer_config);

#endif
