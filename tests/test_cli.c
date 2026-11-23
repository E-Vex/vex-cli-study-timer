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
    CHECK(cli_parse_value("1.5", 255, &value) == 0);
    CHECK(cli_parse_value("1e3", 255, &value) == 0);
    CHECK(cli_parse_value("0x10", 255, &value) == 0);
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
    char *help_after_options[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-h"};
    timer_config_t config = {0};

    CHECK(parse_timer_args(ARGC(help_short), help_short, &config) == CLI_RESULT_HELP);
    CHECK(parse_timer_args(ARGC(help_long), help_long, &config) == CLI_RESULT_HELP);
    CHECK(parse_timer_args(ARGC(version_short), version_short, &config) == CLI_RESULT_VERSION);
    CHECK(parse_timer_args(ARGC(version_long), version_long, &config) == CLI_RESULT_VERSION);

    /* Help still wins when it appears after other options. */
    CHECK(parse_timer_args(ARGC(help_after_options), help_after_options, &config) == CLI_RESULT_HELP);
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
    CHECK(vext_alarm_is_valid("hard-to-say-goodbye"));
    CHECK(vext_alarm_is_valid("cinematic"));
    CHECK(vext_alarm_is_valid("garden-guardian"));
    CHECK(vext_alarm_is_valid("final-showdown"));
    CHECK(vext_alarm_is_valid("victory"));
    CHECK(vext_alarm_is_valid("none"));

    CHECK(!vext_alarm_is_valid("bogus"));
    CHECK(!vext_alarm_is_valid(""));
    CHECK(!vext_alarm_is_valid(NULL));

    CHECK(vext_alarm_is_none("none"));
    CHECK(!vext_alarm_is_none("megalovania"));
    CHECK(!vext_alarm_is_none(NULL));
}

static void test_inline_value_options(void)
{
    char *eq_full[] = {"vex-timer", "--sessions=4", "--focus=25", "--break=5", "--alarm=tetris"};
    char *eq_mixed[] = {"vex-timer", "-s", "4", "--focus=25", "-b", "5"};
    char *eq_none[] = {"vex-timer", "--sessions=1", "--focus=50", "--break=10", "--alarm=none"};
    char *eq_empty[] = {"vex-timer", "--sessions=", "--focus=25", "--break=5"};
    char *eq_bad[] = {"vex-timer", "--sessions=4x", "--focus=25", "--break=5"};
    char *eq_zero[] = {"vex-timer", "--sessions=4", "--focus=0", "--break=5"};
    char *eq_overflow[] = {"vex-timer", "--sessions=4", "--focus=25", "--break=256"};
    char *eq_unknown[] = {"vex-timer", "--nope=1"};
    char *eq_help[] = {"vex-timer", "--help=1"};
    char *eq_version[] = {"vex-timer", "--version=1"};
    char *short_eq[] = {"vex-timer", "-s=4", "-t", "25", "-b", "5"};
    timer_config_t config = {0};

    CHECK(parse_timer_args(ARGC(eq_full), eq_full, &config) == CLI_RESULT_OK);
    CHECK(config.session == 4);
    CHECK(config.session_time == 25);
    CHECK(config.break_time == 5);
    CHECK_STRING(config.alarm, "tetris");

    memset(&config, 0, sizeof(config));
    CHECK(parse_timer_args(ARGC(eq_mixed), eq_mixed, &config) == CLI_RESULT_OK);
    CHECK(config.session == 4);
    CHECK(config.session_time == 25);
    CHECK(config.break_time == 5);

    memset(&config, 0, sizeof(config));
    CHECK(parse_timer_args(ARGC(eq_none), eq_none, &config) == CLI_RESULT_OK);
    CHECK(config.session == 1);
    CHECK(vext_alarm_is_none(config.alarm));

    CHECK(parse_quiet(ARGC(eq_empty), eq_empty, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(eq_bad), eq_bad, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(eq_zero), eq_zero, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(eq_overflow), eq_overflow, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(eq_unknown), eq_unknown, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(eq_help), eq_help, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(eq_version), eq_version, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(short_eq), short_eq, &config) == CLI_RESULT_ERROR);
}

static void test_long_break_options(void)
{
    char *lb_default[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-l", "15"};
    char *lb_every[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-l", "15", "-r", "2"};
    char *lb_inline[] = {"vex-timer", "--sessions=8", "--focus=25", "--break=5",
                         "--long-break=30", "--long-break-every=4"};
    char *lb_reorder[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-r", "2", "-l", "15"};
    char *lb_disabled[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5"};
    char *lb_r_only[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-r", "4"};
    char *lb_r_only_inline[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5",
                                "--long-break-every=4"};
    char *lb_zero[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-l", "0"};
    char *lb_over[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-l", "256"};
    char *lb_dup[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-l", "15", "-l", "20"};
    char *lb_every_zero[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-l", "15", "-r", "0"};
    char *lb_every_over[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-l", "15", "-r", "256"};
    char *lb_missing_value[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "-l"};
    char *lb_empty_inline[] = {"vex-timer", "-s", "4", "-t", "25", "-b", "5", "--long-break="};
    timer_config_t config = {0};

    CHECK(parse_timer_args(ARGC(lb_default), lb_default, &config) == CLI_RESULT_OK);
    CHECK(config.long_break_time == 15);
    CHECK(config.long_break_every == VEXT_DEFAULT_LONG_BREAK_EVERY);

    memset(&config, 0, sizeof(config));
    CHECK(parse_timer_args(ARGC(lb_every), lb_every, &config) == CLI_RESULT_OK);
    CHECK(config.long_break_time == 15);
    CHECK(config.long_break_every == 2);

    memset(&config, 0, sizeof(config));
    CHECK(parse_timer_args(ARGC(lb_inline), lb_inline, &config) == CLI_RESULT_OK);
    CHECK(config.long_break_time == 30);
    CHECK(config.long_break_every == 4);

    /* -r before -l is accepted; only the pair matters, not the order. */
    memset(&config, 0, sizeof(config));
    CHECK(parse_timer_args(ARGC(lb_reorder), lb_reorder, &config) == CLI_RESULT_OK);
    CHECK(config.long_break_time == 15);
    CHECK(config.long_break_every == 2);

    memset(&config, 0, sizeof(config));
    CHECK(parse_timer_args(ARGC(lb_disabled), lb_disabled, &config) == CLI_RESULT_OK);
    CHECK(config.long_break_time == 0);

    CHECK(parse_quiet(ARGC(lb_r_only), lb_r_only, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(lb_r_only_inline), lb_r_only_inline, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(lb_zero), lb_zero, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(lb_over), lb_over, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(lb_dup), lb_dup, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(lb_every_zero), lb_every_zero, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(lb_every_over), lb_every_over, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(lb_missing_value), lb_missing_value, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(lb_empty_inline), lb_empty_inline, &config) == CLI_RESULT_ERROR);
}

static void test_no_notify_option(void)
{
    char *default_args[] = {"vex-timer", "-s", "1", "-t", "25", "-b", "5"};
    char *no_notify[] = {"vex-timer", "-s", "1", "-t", "25", "-b", "5", "--no-notify"};
    char *no_notify_dup[] = {"vex-timer", "-s", "1", "-t", "25", "-b", "5",
                             "--no-notify", "--no-notify"};
    char *no_notify_value[] = {"vex-timer", "-s", "1", "-t", "25", "-b", "5", "--no-notify=1"};
    timer_config_t config = {0};

    CHECK(parse_timer_args(ARGC(default_args), default_args, &config) == CLI_RESULT_OK);
    CHECK(config.notify == 1);

    memset(&config, 0, sizeof(config));
    CHECK(parse_timer_args(ARGC(no_notify), no_notify, &config) == CLI_RESULT_OK);
    CHECK(config.notify == 0);

    CHECK(parse_quiet(ARGC(no_notify_dup), no_notify_dup, &config) == CLI_RESULT_ERROR);
    CHECK(parse_quiet(ARGC(no_notify_value), no_notify_value, &config) == CLI_RESULT_ERROR);
}

void test_cli(void)
{
    test_parse_value();
    test_valid_options();
    test_information_options();
    test_invalid_options();
    test_inline_value_options();
    test_long_break_options();
    test_no_notify_option();
    test_alarm_names();
}
