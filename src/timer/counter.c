#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <time.h>

#include "../input/input.h"
#include "counter.h"

#define VEXT_PAUSE_POLL_NS 200000000ULL /* 200 ms */

uint64_t vext_now_ns(void)
{
    struct timespec time_spec;

    if (clock_gettime(CLOCK_MONOTONIC, &time_spec) != 0)
        return 0;

    return (uint64_t)time_spec.tv_sec * VEXT_NS_PER_SECOND +
           (uint64_t)time_spec.tv_nsec;
}

/*
 * The countdown is based on an absolute CLOCK_MONOTONIC deadline rather than on
 * accumulating sleeps. This keeps it accurate even when a tick is delayed by
 * terminal output, alarm handling, or an interrupted sleep.
 *
 * While a phase runs, this function waits for either the next displayed second
 * or a key press. Pausing freezes the remaining nanoseconds; resuming rebuilds
 * the deadline from that frozen value, so no time is gained or lost.
 */
vext_counter_status_t vext_counter_v2(timer_config_t *timer_config)
{
    uint64_t now_ns = vext_now_ns();
    uint64_t remaining_ns = 0;
    uint64_t remaining_seconds;
    uint64_t wait_ns;
    vext_key_t key;

    if (timer_config->paused)
        remaining_ns = timer_config->paused_remaining_ns;
    else if (timer_config->target_end_ns > now_ns)
        remaining_ns = timer_config->target_end_ns - now_ns;

    /* Round up so a fresh 25-minute phase starts by displaying 25:00. */
    remaining_seconds = (remaining_ns + VEXT_NS_PER_SECOND - 1) / VEXT_NS_PER_SECOND;
    timer_config->min = (uint32_t)(remaining_seconds / 60);
    timer_config->sec = (uint32_t)(remaining_seconds % 60);

    if (remaining_ns == 0)
    {
        timer_config->paused = 0;
        return VEXT_COUNTER_DONE;
    }

    if (timer_config->paused)
    {
        wait_ns = VEXT_PAUSE_POLL_NS;
        if (timer_config->paused_remaining_ns < wait_ns)
            wait_ns = timer_config->paused_remaining_ns;
    }
    else
    {
        /* Wake up when the displayed second changes. */
        wait_ns = remaining_ns % VEXT_NS_PER_SECOND;
        if (wait_ns == 0)
            wait_ns = VEXT_NS_PER_SECOND;
    }

    key = vext_input_poll(wait_ns);

    if (key == VEXT_KEY_QUIT)
        return VEXT_COUNTER_QUIT;

    if (key == VEXT_KEY_SKIP)
    {
        timer_config->paused = 0;
        return VEXT_COUNTER_SKIPPED;
    }

    if (key == VEXT_KEY_PAUSE)
    {
        if (timer_config->paused)
        {
            timer_config->target_end_ns = vext_now_ns() +
                                          timer_config->paused_remaining_ns;
            timer_config->paused = 0;
        }
        else
        {
            uint64_t pause_now = vext_now_ns();

            timer_config->paused_remaining_ns =
                timer_config->target_end_ns > pause_now
                    ? timer_config->target_end_ns - pause_now
                    : 0;
            timer_config->paused = 1;
        }
    }

    return VEXT_COUNTER_RUNNING;
}
