#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../input/input.h"
#include "../notify/notify.h"
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
        timer_config->break_time == 0 || timer_config->break_time > VEXT_MAX_MINUTES ||
        timer_config->long_break_time > VEXT_MAX_MINUTES)
    {
        return VEXT_STATUS_OUT_OF_RANGE;
    }

    if (timer_config->long_break_time > 0 &&
        (timer_config->long_break_every == 0 ||
         timer_config->long_break_every > VEXT_MAX_SESSIONS))
    {
        return VEXT_STATUS_OUT_OF_RANGE;
    }

    return VEXT_STATUS_VALID;
}

int vext_is_long_break(const timer_config_t *timer_config, uint32_t completed_sessions)
{
    if (timer_config == NULL)
        return 0;

    if (timer_config->long_break_time == 0 || timer_config->long_break_every == 0)
        return 0;

    return completed_sessions % timer_config->long_break_every == 0;
}

void vext_plan_summary(const timer_config_t *timer_config, vext_plan_summary_t *summary)
{
    unsigned long completed_breaks;
    unsigned long long_breaks;

    if (timer_config == NULL || summary == NULL)
        return;

    summary->focus_minutes = (unsigned long)timer_config->session * timer_config->session_time;

    /* Breaks only run between sessions, never after the final one. */
    completed_breaks = timer_config->session > 0 ? (unsigned long)timer_config->session - 1 : 0;
    long_breaks = (timer_config->long_break_time > 0 && timer_config->long_break_every > 0)
                      ? completed_breaks / timer_config->long_break_every
                      : 0;

    summary->break_minutes =
        (completed_breaks - long_breaks) * (unsigned long)timer_config->break_time;
    summary->long_break_minutes = long_breaks * (unsigned long)timer_config->long_break_time;
    summary->total_minutes =
        summary->focus_minutes + summary->break_minutes + summary->long_break_minutes;
}

static void display_phase_header(const timer_config_t *timer_config)
{
    printf("\033[H\033[J"); /* Clear the screen */

    display_banner();
    display_info(timer_config);

    if (vext_input_is_interactive())
        display_controls();
}

/* Which kind of phase run_phase() is counting down. */
typedef enum
{
    PHASE_FOCUS = 0,
    PHASE_BREAK,
    PHASE_LONG_BREAK
} phase_kind_t;

static uint_fast8_t phase_validator_status(phase_kind_t kind)
{
    switch (kind)
    {
    case PHASE_BREAK:
        return VEXT_STATUS_BREAK_PHASE;

    case PHASE_LONG_BREAK:
        return VEXT_STATUS_LONG_BREAK;

    case PHASE_FOCUS:
    default:
        return VEXT_STATUS_VALID;
    }
}

/*
 * Run one study or break phase until it finishes, is skipped, or the user
 * quits. The deadline lives in the timer config so pause/resume can rebuild it
 * without accumulating sleep drift.
 */
static uint_fast8_t run_phase(timer_config_t *timer_config, uint32_t duration_minutes,
                              phase_kind_t kind)
{
    timer_config->sec = 0;
    timer_config->min = duration_minutes;
    timer_config->paused = 0;
    timer_config->paused_remaining_ns = 0;
    timer_config->validator_status = phase_validator_status(kind);
    timer_config->target_end_ns = vext_now_ns() +
                                  (uint64_t)duration_minutes * VEXT_NS_PER_MINUTE;

    while (1)
    {
        vext_printer(timer_config);

        switch (vext_counter_v2(timer_config))
        {
        case VEXT_COUNTER_DONE:
            vext_printer(timer_config);
            return kind == PHASE_FOCUS ? VEXT_STATUS_STUDY_DONE : VEXT_STATUS_BREAK_DONE;

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

    vext_notify_set_enabled(timer_config->notify != 0);

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
        uint32_t completed_session;
        int long_break;
        uint32_t break_duration;
        phase_kind_t break_kind;

        if (vext_input_interrupted() || vext_input_quit_requested())
            return VEXT_EXIT_INTERRUPTED;

        completed_session = timer_config->ses;

        study_status = run_phase(timer_config, timer_config->session_time, PHASE_FOCUS);
        if (study_status == VEXT_STATUS_QUIT)
            return VEXT_EXIT_INTERRUPTED;

        timer_config->ses += 1;
        timer_config->validator_status = vext_validator(timer_config);

        if (timer_config->validator_status == VEXT_STATUS_FINISH)
            break;

        /* Skipped sessions still count, so they still advance the cadence. */
        long_break = vext_is_long_break(timer_config, completed_session);
        break_duration = long_break ? timer_config->long_break_time : timer_config->break_time;
        break_kind = long_break ? PHASE_LONG_BREAK : PHASE_BREAK;

        if (study_status != VEXT_STATUS_SKIPPED)
        {
            char body[128];

            snprintf(body, sizeof(body), "Session %u of %u done. Time for a %sbreak.",
                     completed_session, timer_config->session,
                     long_break ? "long " : "");
            vext_notify("Focus session complete", body);

            vext_alarm(timer_config->alarm);
            if (vext_input_interrupted() || vext_input_quit_requested())
                return VEXT_EXIT_INTERRUPTED;
            display_phase_header(timer_config);
        }

        break_status = run_phase(timer_config, break_duration, break_kind);
        if (break_status == VEXT_STATUS_QUIT)
            return VEXT_EXIT_INTERRUPTED;

        if (break_status != VEXT_STATUS_SKIPPED)
        {
            char body[128];

            snprintf(body, sizeof(body), "Time to focus. Session %u of %u.",
                     timer_config->ses, timer_config->session);
            vext_notify("Break over", body);

            vext_alarm(timer_config->alarm);
            if (vext_input_interrupted() || vext_input_quit_requested())
                return VEXT_EXIT_INTERRUPTED;
            display_phase_header(timer_config);
        }
    }

    vext_printer(timer_config); /* Finish screen */

    {
        char body[128];
        vext_plan_summary_t summary;

        vext_plan_summary(timer_config, &summary);
        snprintf(body, sizeof(body), "%u focus sessions, %lu minutes of focus time.",
                 timer_config->session, summary.focus_minutes);
        vext_notify("All sessions complete", body);
    }

    if (!vext_alarm_is_none(timer_config->alarm))
    {
        play_finish_music();

        if (vext_input_interrupted() || vext_input_quit_requested())
            return VEXT_EXIT_INTERRUPTED;
    }

    return EXIT_SUCCESS;
}
