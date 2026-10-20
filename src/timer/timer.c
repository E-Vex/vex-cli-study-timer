#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

uint_fast8_t vext_study(timer_config_t *timer_config)
{
    timer_config->sec = 0;
    timer_config->min = timer_config->session_time;
    timer_config->validator_status = VEXT_STATUS_VALID;

    timer_config->target_end_ns = vext_now_ns() +
                                  (uint64_t)timer_config->session_time * VEXT_NS_PER_MINUTE;

    while (1)
    {
        vext_printer(timer_config);

        if (vext_counter_v2(timer_config) == 0x0)
        {
            vext_printer(timer_config);
            break;
        }
    }

    timer_config->ses += 1;

    if (vext_validator(timer_config) == VEXT_STATUS_BREAK_PHASE)
    {
        vext_alarm();
        printf("\033[H\033[J");
        display_banner();
        display_info(timer_config);
    }

    return VEXT_STATUS_STUDY_DONE; /* End of a study session */
}

uint_fast8_t vext_break(timer_config_t *timer_config)
{
    timer_config->sec = 0;
    timer_config->min = timer_config->break_time;
    timer_config->validator_status = VEXT_STATUS_BREAK_PHASE;

    timer_config->target_end_ns = vext_now_ns() +
                                  (uint64_t)timer_config->break_time * VEXT_NS_PER_MINUTE;

    while (1)
    {
        vext_printer(timer_config);

        if (vext_counter_v2(timer_config) == 0x0)
        {
            vext_printer(timer_config);
            break;
        }
    }

    vext_alarm();
    printf("\033[H\033[J");
    display_banner();
    display_info(timer_config);

    return VEXT_STATUS_BREAK_DONE; /* End of a break */
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

int vext_controller(timer_config_t *timer_config)
{
    timer_config->ses = 1;
    timer_config->validator_status = VEXT_STATUS_VALID;

    if (vext_input_validator(timer_config) == VEXT_STATUS_OUT_OF_RANGE)
    {
        fprintf(stderr, "E: session count and durations must be between 1 and %u\n",
                VEXT_MAX_MINUTES);
        return EXIT_FAILURE;
    }

    printf("\033[H\033[J"); /* Clear the screen */

    display_banner();
    display_info(timer_config);

    while (timer_config->validator_status != VEXT_STATUS_FINISH)
    {
        vext_study(timer_config);

        timer_config->validator_status = vext_validator(timer_config);
        if (timer_config->validator_status == VEXT_STATUS_BREAK_PHASE)
        {
            vext_break(timer_config);
        }
    }

    vext_printer(timer_config);
    play_finish_music();

    return EXIT_SUCCESS;
}
