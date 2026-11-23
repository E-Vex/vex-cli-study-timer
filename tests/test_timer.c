#include "../src/timer/counter.h"
#include "../src/timer/timer.h"
#include "test_util.h"

static timer_config_t make_config(void)
{
    timer_config_t config = {0};

    config.session = 4;
    config.session_time = 25;
    config.break_time = 5;
    config.alarm = "megalovania";
    config.ses = 1;

    return config;
}

static void test_input_validator(void)
{
    timer_config_t config = make_config();

    CHECK(vext_input_validator(&config) == VEXT_STATUS_VALID);

    config.session = 0;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_OUT_OF_RANGE);

    config = make_config();
    config.session = 256;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_OUT_OF_RANGE);

    config = make_config();
    config.session_time = 0;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_OUT_OF_RANGE);

    config = make_config();
    config.session_time = 256;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_OUT_OF_RANGE);

    config = make_config();
    config.break_time = 0;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_OUT_OF_RANGE);

    config = make_config();
    config.break_time = 256;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_OUT_OF_RANGE);

    config = make_config();
    config.long_break_time = 256;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_OUT_OF_RANGE);

    config = make_config();
    config.long_break_time = 15;
    config.long_break_every = 0;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_OUT_OF_RANGE);

    config = make_config();
    config.long_break_time = 15;
    config.long_break_every = 256;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_OUT_OF_RANGE);

    config = make_config();
    config.session = VEXT_MAX_SESSIONS;
    config.session_time = VEXT_MAX_MINUTES;
    config.break_time = VEXT_MAX_MINUTES;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_VALID);

    config.long_break_time = VEXT_MAX_MINUTES;
    config.long_break_every = VEXT_MAX_SESSIONS;
    CHECK(vext_input_validator(&config) == VEXT_STATUS_VALID);
}

static void test_session_validator(void)
{
    timer_config_t config = make_config();

    config.ses = 1;
    CHECK(vext_validator(&config) == VEXT_STATUS_BREAK_PHASE);

    config.ses = 4;
    CHECK(vext_validator(&config) == VEXT_STATUS_BREAK_PHASE);

    config.ses = 5;
    CHECK(vext_validator(&config) == VEXT_STATUS_FINISH);
}

static void test_counter_deadline(void)
{
    timer_config_t config = make_config();
    uint64_t before;
    uint64_t after;

    config.target_end_ns = 0; /* long in the past */
    config.paused = 0;

    CHECK(vext_counter_v2(&config) == VEXT_COUNTER_DONE);
    CHECK(config.min == 0);
    CHECK(config.sec == 0);

    before = vext_now_ns();
    after = vext_now_ns();

    CHECK(before > 0);
    CHECK(after >= before);
}

static void test_is_long_break(void)
{
    timer_config_t config = make_config();

    config.long_break_time = 15;
    config.long_break_every = 4;

    CHECK(!vext_is_long_break(&config, 1));
    CHECK(!vext_is_long_break(&config, 2));
    CHECK(!vext_is_long_break(&config, 3));
    CHECK(vext_is_long_break(&config, 4));
    CHECK(!vext_is_long_break(&config, 5));
    CHECK(!vext_is_long_break(&config, 7));
    CHECK(vext_is_long_break(&config, 8));

    config.long_break_time = 0; /* long breaks disabled */
    CHECK(!vext_is_long_break(&config, 4));
    CHECK(!vext_is_long_break(&config, 8));

    config.long_break_time = 15;
    config.long_break_every = 1; /* every break is a long break */
    CHECK(vext_is_long_break(&config, 1));

    config.long_break_every = 0; /* invalid interval is never a long break */
    CHECK(!vext_is_long_break(&config, 4));

    CHECK(!vext_is_long_break(NULL, 4));
}

static void test_plan_summary(void)
{
    timer_config_t config = make_config();
    vext_plan_summary_t summary;

    vext_plan_summary(&config, &summary);
    CHECK(summary.focus_minutes == 100);
    CHECK(summary.break_minutes == 15);
    CHECK(summary.long_break_minutes == 0);
    CHECK(summary.total_minutes == 115);

    /* The long break would fall after the last session, so it never runs. */
    config.long_break_time = 15;
    config.long_break_every = 4;
    vext_plan_summary(&config, &summary);
    CHECK(summary.focus_minutes == 100);
    CHECK(summary.break_minutes == 15);
    CHECK(summary.long_break_minutes == 0);
    CHECK(summary.total_minutes == 115);

    /* One long break after session 4, short breaks after 1-3. */
    config.session = 5;
    vext_plan_summary(&config, &summary);
    CHECK(summary.focus_minutes == 125);
    CHECK(summary.break_minutes == 15);
    CHECK(summary.long_break_minutes == 15);
    CHECK(summary.total_minutes == 155);

    /* Long breaks after sessions 4 and 8, short breaks after the rest. */
    config.session = 9;
    vext_plan_summary(&config, &summary);
    CHECK(summary.focus_minutes == 225);
    CHECK(summary.break_minutes == 30);
    CHECK(summary.long_break_minutes == 30);
    CHECK(summary.total_minutes == 285);

    vext_plan_summary(&config, NULL); /* must not crash */
    vext_plan_summary(NULL, &summary); /* must not crash */
}

void test_timer(void)
{
    test_input_validator();
    test_session_validator();
    test_counter_deadline();
    test_is_long_break();
    test_plan_summary();
}
