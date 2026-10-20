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
    config.session = VEXT_MAX_SESSIONS;
    config.session_time = VEXT_MAX_MINUTES;
    config.break_time = VEXT_MAX_MINUTES;
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

void test_timer(void)
{
    test_input_validator();
    test_session_validator();
    test_counter_deadline();
}
