#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../input/input.h"
#include "alarm.h"
#include "counter.h"
#include "print.h"
#include "../../sounds/tone.h"

#include "timer.h"

uint_fast8_t vext_validator(timer_config_t *timer_config)
{
    if (timer_config->ses > timer_config->session)
        return VEXT_STATUS_FINISH; /* Finish: end of all sessions */

    return VEXT_STATUS_BREAK_PHASE; /* Break phase: end of a study session */
}

uint_fast8_t vext_input_validator(timer_config_t *timer_config)
{
    if (timer_config->session == 0 || timer_config->session > VEXT_MAX_SESSIONS ||
        timer_config->session_time == 0 || timer_config->session_time > VEXT_MAX_MINUTES ||
        timer_config->break_time == 0 || timer_config->break_time > VEXT_MAX_MINUTES)
    {
        return VEXT_STATUS_OUT_OF_RANGE;
    }

    return VEXT_STATUS_VALID;
}

static void display_phase_header(const timer_config_t *timer_config)
{
    printf("\033[H\033[J"); /* Clear the screen */

    display_banner();
    display_info(timer_config);

    if (vext_input_is_interactive())
        display_controls();
}

/*
 * Run one study or break phase until it finishes, is skipped, or the user
 * quits. The deadline lives in the timer config so pause/resume can rebuild it
 * without accumulating sleep drift.
 */
static uint_fast8_t run_phase(timer_config_t *timer_config, uint32_t duration_minutes, int is_break)
{
    timer_config->sec = 0;
    timer_config->min = duration_minutes;
    timer_config->paused = 0;
    timer_config->paused_remaining_ns = 0;
    timer_config->validator_status = is_break ? VEXT_STATUS_BREAK_PHASE : VEXT_STATUS_VALID;
    timer_config->target_end_ns = vext_now_ns() +
                                  (uint64_t)duration_minutes * VEXT_NS_PER_MINUTE;

    while (1)
    {
        vext_printer(timer_config);

        switch (vext_counter_v2(timer_config))
        {
        case VEXT_COUNTER_DONE:
            vext_printer(timer_config);
            return is_break ? VEXT_STATUS_BREAK_DONE : VEXT_STATUS_STUDY_DONE;

        case VEXT_COUNTER_SKIPPED:
            return VEXT_STATUS_SKIPPED;

        case VEXT_COUNTER_QUIT:
            return VEXT_STATUS_QUIT;

        case VEXT_COUNTER_RUNNING:
        default:
            break;
        }
    }
}

int vext_controller(timer_config_t *timer_config)
{
    if (timer_config->alarm == NULL)
        timer_config->alarm = VEXT_DEFAULT_ALARM;

    timer_config->ses = 1;
    timer_config->paused = 0;
    timer_config->validator_status = VEXT_STATUS_VALID;

    if (vext_input_validator(timer_config) == VEXT_STATUS_OUT_OF_RANGE)
    {
        fprintf(stderr, "E: session count and durations must be between 1 and %u\n",
                VEXT_MAX_MINUTES);
        return EXIT_FAILURE;
    }

    display_phase_header(timer_config);

    while (1)
    {
        uint_fast8_t study_status;
        uint_fast8_t break_status;

        if (vext_input_interrupted() || vext_input_quit_requested())
            return VEXT_EXIT_INTERRUPTED;

        study_status = run_phase(timer_config, timer_config->session_time, 0);
        if (study_status == VEXT_STATUS_QUIT)
            return VEXT_EXIT_INTERRUPTED;

        timer_config->ses += 1;
        timer_config->validator_status = vext_validator(timer_config);

        if (timer_config->validator_status == VEXT_STATUS_FINISH)
            break;

        if (study_status != VEXT_STATUS_SKIPPED)
        {
            vext_alarm(timer_config->alarm);
            if (vext_input_interrupted() || vext_input_quit_requested())
                return VEXT_EXIT_INTERRUPTED;
            display_phase_header(timer_config);
        }

        break_status = run_phase(timer_config, timer_config->break_time, 1);
        if (break_status == VEXT_STATUS_QUIT)
            return VEXT_EXIT_INTERRUPTED;

        if (break_status != VEXT_STATUS_SKIPPED)
        {
            vext_alarm(timer_config->alarm);
            if (vext_input_interrupted() || vext_input_quit_requested())
                return VEXT_EXIT_INTERRUPTED;
            display_phase_header(timer_config);
        }
    }

    vext_printer(timer_config); /* Finish screen */

    if (!vext_alarm_is_none(timer_config->alarm))
    {
        play_finish_music();

        if (vext_input_interrupted() || vext_input_quit_requested())
            return VEXT_EXIT_INTERRUPTED;
    }

    return EXIT_SUCCESS;
}
