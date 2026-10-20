#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "../timer/alarm.h"
#include "../version.h"

int cli_parse_value(const char *text, uint32_t max, uint32_t *out)
{
    const char *cursor;
    char *end = NULL;
    long value;

    if (text == NULL || *text == '\0')
        return 0;

    /* Accept digits only: no signs, whitespace, or trailing garbage. */
    for (cursor = text; *cursor != '\0'; cursor++)
    {
        if (*cursor < '0' || *cursor > '9')
            return 0;
    }

    errno = 0;
    value = strtol(text, &end, 10);

    if (errno == ERANGE || end == text || *end != '\0')
        return 0;

    if (value < 1 || (unsigned long)value > (unsigned long)max)
        return 0;

    *out = (uint32_t)value;
    return 1;
}

void print_usage(FILE *stream)
{
    fprintf(stream,
            "vex-timer - a command-line study timer\n"
            "\n"
            "Usage:\n"
            "  vex-timer -s <sessions> -t <focus-minutes> -b <break-minutes> [-a <alarm>]\n"
            "  vex-timer -h | --help\n"
            "  vex-timer -v | --version\n"
            "\n"
            "Options:\n"
            "  -s, --sessions N   number of study sessions (1-%u, required)\n"
            "  -t, --focus N      study duration in minutes (1-%u, required)\n"
            "  -b, --break N      break duration in minutes (1-%u, required)\n"
            "  -a, --alarm NAME   alarm sound (default: %s)\n"
            "  -h, --help         show this help message\n"
            "  -v, --version      show version information\n"
            "\n"
            "Alarm sounds:\n"
            "  ",
            VEXT_MAX_SESSIONS, VEXT_MAX_MINUTES, VEXT_MAX_MINUTES, VEXT_DEFAULT_ALARM);

    vext_alarm_print_names(stream);

    fprintf(stream,
            "\n"
            "\n"
            "Controls:\n"
            "  Space/P  pause or resume the current phase\n"
            "  S        skip the current phase\n"
            "  Q        quit immediately\n"
            "  Any key  stop a ringing alarm\n"
            "  Ctrl+C   quit\n"
            "\n"
            "Example:\n"
            "  vex-timer -s 4 -t 25 -b 5 -a tetris\n");
}

void print_version(void)
{
    printf("vex-timer %s\n", VEX_VERSION);
}

cli_result_t parse_timer_args(int argc, char *argv[], timer_config_t *timer_config)
{
    int seen_sessions = 0;
    int seen_focus = 0;
    int seen_break = 0;
    int seen_alarm = 0;

    timer_config->alarm = VEXT_DEFAULT_ALARM;

    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        uint32_t *value = NULL;
        int *seen = NULL;
        uint32_t max = 0;
        const char *label = NULL;

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
            return CLI_RESULT_HELP;

        if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0)
            return CLI_RESULT_VERSION;

        if (strcmp(arg, "-a") == 0 || strcmp(arg, "--alarm") == 0)
        {
            if (seen_alarm)
            {
                fprintf(stderr, "E: option '%s' was specified more than once\n", arg);
                return CLI_RESULT_ERROR;
            }

            if (i + 1 >= argc)
            {
                fprintf(stderr, "E: option '%s' requires a value\n", arg);
                print_usage(stderr);
                return CLI_RESULT_ERROR;
            }

            if (!vext_alarm_is_valid(argv[i + 1]))
            {
                fprintf(stderr, "E: unknown alarm '%s' (available: ", argv[i + 1]);
                vext_alarm_print_names(stderr);
                fprintf(stderr, ")\n");
                return CLI_RESULT_ERROR;
            }

            timer_config->alarm = argv[i + 1];
            seen_alarm = 1;
            i++;
            continue;
        }

        if (strcmp(arg, "-s") == 0 || strcmp(arg, "--sessions") == 0)
        {
            value = &timer_config->session;
            seen = &seen_sessions;
            max = VEXT_MAX_SESSIONS;
            label = "session count";
        }
        else if (strcmp(arg, "-t") == 0 || strcmp(arg, "--focus") == 0)
        {
            value = &timer_config->session_time;
            seen = &seen_focus;
            max = VEXT_MAX_MINUTES;
            label = "focus duration";
        }
        else if (strcmp(arg, "-b") == 0 || strcmp(arg, "--break") == 0)
        {
            value = &timer_config->break_time;
            seen = &seen_break;
            max = VEXT_MAX_MINUTES;
            label = "break duration";
        }
        else
        {
            fprintf(stderr, "E: unknown option '%s'\n", arg);
            print_usage(stderr);
            return CLI_RESULT_ERROR;
        }

        if (*seen)
        {
            fprintf(stderr, "E: option '%s' was specified more than once\n", arg);
            return CLI_RESULT_ERROR;
        }

        if (i + 1 >= argc)
        {
            fprintf(stderr, "E: option '%s' requires a value\n", arg);
            print_usage(stderr);
            return CLI_RESULT_ERROR;
        }

        if (!cli_parse_value(argv[i + 1], max, value))
        {
            fprintf(stderr, "E: invalid %s '%s' (expected an integer from 1 to %u)\n",
                    label, argv[i + 1], max);
            return CLI_RESULT_ERROR;
        }

        *seen = 1;
        i++;
    }

    if (!seen_sessions || !seen_focus || !seen_break)
    {
        fprintf(stderr, "E: missing required option(s):");
        if (!seen_sessions)
            fprintf(stderr, " -s/--sessions");
        if (!seen_focus)
            fprintf(stderr, " -t/--focus");
        if (!seen_break)
            fprintf(stderr, " -b/--break");
        fprintf(stderr, "\n");
        print_usage(stderr);
        return CLI_RESULT_ERROR;
    }

    return CLI_RESULT_OK;
}
