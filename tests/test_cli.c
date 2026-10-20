#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../src/cli/cli.h"
#include "../src/timer/alarm.h"
#include "test_util.h"

#define ARGC(array) ((int)(sizeof(array) / sizeof((array)[0])))

static int saved_stderr_fd = -1;

static void silence_stderr(void)
{
    int dev_null;

    fflush(stderr);
    saved_stderr_fd = dup(STDERR_FILENO);
    dev_null = open("/dev/null", O_WRONLY);

    if (dev_null >= 0)
    {
        dup2(dev_null, STDERR_FILENO);
        close(dev_null);
    }
}

static void restore_stderr(void)
{
    fflush(stderr);

    if (saved_stderr_fd >= 0)
    {
        dup2(saved_stderr_fd, STDERR_FILENO);
        close(saved_stderr_fd);
        saved_stderr_fd = -1;
    }
}

static cli_result_t parse_quiet(int argc, char *argv[], timer_config_t *config)
{
    cli_result_t result;

    silence_stderr();
    result = parse_timer_args(argc, argv, config);
    restore_stderr();

    return result;
}

static void test_parse_value(void)
{
    uint32_t value = 0;

    CHECK(cli_parse_value("1", 255, &value) == 1);
    CHECK(value == 1);

    CHECK(cli_parse_value("255", 255, &value) == 1);
    CHECK(value == 255);

    CHECK(cli_parse_value("007", 255, &value) == 1);
    CHECK(value == 7);

    value = 0;
    CHECK(cli_parse_value("0", 255, &value) == 0);
    CHECK(cli_parse_value("256", 255, &value) == 0);
    CHECK(cli_parse_value("", 255, &value) == 0);
    CHECK(cli_parse_value(NULL, 255, &value) == 0);
    CHECK(cli_parse_value("-1", 255, &value) == 0);
    CHECK(cli_parse_value("+1", 255, &value) == 0);
    CHECK(cli_parse_value(" 1", 255, &value) == 0);
    CHECK(cli_parse_value("1 ", 255, &value) == 0);
    CHECK(cli_parse_value("1x", 255, &value) == 0);
    CHECK(cli_parse_value("abc", 255, &value) == 0);
    CHECK(cli_parse_value("999999999999999999999", 255, &value) == 0);
    CHECK(value == 0);
}

static void test_valid_options(void)
{
    char *short_args[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5"};
    char *long_args[] = {"vex-timer", "--sessions", "3", "--focus", "50", "--break", "10"};
    char *alarm_args[] = {"vex-timer", "-s", "1", "-t", "25", "-b", "5", "-a", "tetris"};
    char *none_args[] = {"vex-timer", "-s", "1", "-t", "25", "-b", "5", "--alarm", "none"};
    timer_config_t config = {0};

    CHECK(parse_timer_args(ARGC(short_args), short_args, &config) == CLI_RESULT_OK);
    CHECK(config.session == 4);
    CHECK(config.session_time == 25);
    CHECK(config.break_time == 5);
    CHECK_STRING(config.alarm, VEXT_DEFAULT_ALARM);

    memset(&config, 0, sizeof(config));
    CHECK(parse_timer_args(ARGC(long_args), long_args, &config) == CLI_RESULT_OK);
    CHECK(config.session == 3);
    CHECK(config.session_time == 50);
    CHECK(config.break_time == 10);

    memset(&config, 0, sizeof(config));
    CHECK(parse_timer_args(ARGC(alarm_args), alarm_args, &config) == CLI_RESULT_OK);
    CHECK_STRING(config.alarm, "tetris");

    memset(&config, 0, sizeof(config));
    CHECK(parse_timer_args(ARGC(none_args), none_args, &config) == CLI_RESULT_OK);
    CHECK(vext_alarm_is_none(config.alarm));
}

static void test_information_options(void)
{
    char *help_short[] = {"vex-timer", "-h"};
    char *help_long[] = {"vex-timer", "--help"};
    char *version_short[] = {"vex-timer", "-v"};
    char *version_long[] = {"vex-timer", "--version"};
    timer_config_t config = {0};

    CHECK(parse_timer_args(ARGC(help_short), help_short, &config) == CLI_RESULT_HELP);
    CHECK(parse_timer_args(ARGC(help_long), help_long, &config) == CLI_RESULT_HELP);
    CHECK(parse_timer_args(ARGC(version_short), version_short, &config) == CLI_RESULT_VERSION);
    CHECK(parse_timer_args(ARGC(version_long), version_long, &config) == CLI_RESULT_VERSION);
}

static void test_invalid_options(void)
{
    char *missing[] = {"vex-timer", "-s", "4", "-t", "25"};
    char *duplicate[] = {"vex-timer", "-s", "4", "-s", "5", "-t", "25", "-b", "5"};
    char *unknown[] = {"vex-timer", "--nope"};
    char *missing_value[] = {"vex-timer", "-s"};
    char *zero[] = {"vex-timer", "-s", "0", "-t", "25", "-b", "5"};
    char *overflow[] = {"vex-timer", "-s", "256", "-t", "25", "-b", "5"};
    char *garbage[] = {"vex-timer", "-s", "4x", "-t", "25", "-b", "5"};
    char *bad_alarm[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-a", "bogus"};
    char *alarm_missing_value[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-a"};
    char *duplicate_alarm[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5",
                               "-a", "beep", "-a", "tetris"};
    timer_config_t config = {0};

    CHECK(parse_quiet(ARGC(missing), missing, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(duplicate), duplicate, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(unknown), unknown, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(missing_value), missing_value, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(zero), zero, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(overflow), overflow, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(garbage), garbage, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(bad_alarm), bad_alarm, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(alarm_missing_value), alarm_missing_value, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(duplicate_alarm), duplicate_alarm, &config) == CLI_RESULT_ERROR);
}

static void test_alarm_names(void)
{
    CHECK(vext_alarm_is_valid("megalovania"));
    CHECK(vext_alarm_is_valid("remix"));
    CHECK(vext_alarm_is_valid("jazz"));
    CHECK(vext_alarm_is_valid("call-response"));
    CHECK(vext_alarm_is_valid("pvz"));
    CHECK(vext_alarm_is_valid("tetris"));
    CHECK(vext_alarm_is_valid("mario"));
    CHECK(vext_alarm_is_valid("imperial"));
    CHECK(vext_alarm_is_valid("addictive"));
    CHECK(vext_alarm_is_valid("beep"));
    CHECK(vext_alarm_is_valid("none"));

    CHECK(!vext_alarm_is_valid("bogus"));
    CHECK(!vext_alarm_is_valid(""));
    CHECK(!vext_alarm_is_valid(NULL));

    CHECK(vext_alarm_is_none("none"));
    CHECK(!vext_alarm_is_none("megalovania"));
    CHECK(!vext_alarm_is_none(NULL));
}

void test_cli(void)
{
    test_parse_value();
    test_valid_options();
    test_information_options();
    test_invalid_options();
    test_alarm_names();
}
