#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cli.h"

int is_int(const char *str)
{

    if (*str == '\0')
        return 0;

    if (*str == '+' && *(str + 1) != '\0')
        str++;

    int digit_count = 0;
    while (*str != '\0')
    {
        if (!isdigit((unsigned char)*str))
            return 0;
        digit_count++;
        str++;
    }
    return digit_count <= 9;
}

int parse_timer_args(int argc, char *argv[], timer_config_t *timer_config)
{

    if (argc != 7)
    {
        fprintf(stderr, "E: The vex-timer command requires exactly 6 arguments.\n");
        fprintf(stderr, "Usage: vex-timer -s <session> -t <session_time> -b <break-time>\n");
        return ARGC_ERROR;
    }

    /* TODO: argc == 7 only checks the total arg count, not which flags
     * appeared. Repeating a flag (e.g. "-s 5 -s 6") currently overwrites
     * silently instead of being rejected, and a missing flag (e.g. no -t)
     * isn't caught as long as argc still adds up to 7. Needs per-flag
     * "seen" tracking to fix properly. */
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-s") == 0)
        {
            if (i + 1 < argc && is_int(argv[i + 1]) == 1)
            {
                timer_config->session = atoi(argv[i + 1]);
                i++;
            }
            else
            {
                fprintf(stderr, "E: The session value is not valid\n");
                return SESSION_ERROR;
            }
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            if (i + 1 < argc && is_int(argv[i + 1]) == 1)
            {
                timer_config->session_time = atoi(argv[i + 1]);
                i++;
            }
            else
            {
                fprintf(stderr, "E: The session time value is not valid\n");
                return SESSION_TIME_ERROR;
            }
        }
        else if (strcmp(argv[i], "-b") == 0)
        {
            if (i + 1 < argc && is_int(argv[i + 1]) == 1)
            {
                timer_config->break_time = atoi(argv[i + 1]);
                i++;
            }
            else
            {
                fprintf(stderr, "E: The break time value is not valid\n");
                return BREAK_TIME_ERROR;
            }
        }
        else
        {
            fprintf(stderr, "E: Flag not valid\n");
            fprintf(stderr, "Usage: vex-timer -s <session> -t <session_time> -b <break-time>\n");
            return FLAGS_ERROR;
        }
    }
    return 1;
}
