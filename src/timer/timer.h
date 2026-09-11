#ifndef TIMER_H
#define TIMER_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Status codes returned by vext_validator / vext_study / vext_break /
   vext_input_validator. Replaces the previous raw hex "magic numbers"
   (0x0, 0xA, 0xB, 0xAA, 0xBB, 0x1F) with self-documenting names. */
typedef enum
{
    VEXT_STATUS_VALID        = 0x0,  /* generic OK / valid state            */
    VEXT_STATUS_FINISH       = 0xA,  /* end of all sessions                 */
    VEXT_STATUS_BREAK_PHASE  = 0xB,  /* end of a study session -> break     */
    VEXT_STATUS_STUDY_DONE   = 0xAA, /* a study session has completed       */
    VEXT_STATUS_BREAK_DONE   = 0xBB, /* a break has completed               */
    VEXT_STATUS_OUT_OF_RANGE = 0x1F  /* invalid/out-of-range input          */
} vext_status_t;

typedef struct
{
    uint32_t sec;
    uint32_t min;
    uint32_t remaining_sec;

} started_time_t;

typedef struct
{
    uint32_t sec;
    uint32_t min;
    uint32_t remaining_sec;

} time_now_t;

typedef struct
{
    uint32_t session;      /* total number of study sessions */
    uint32_t session_time; /* duration of each session, in minutes */
    uint32_t break_time;   /* duration of break between sessions, in minutes */
    /*-------------------------------------------------------------------------------*/
    uint32_t sec;
    uint32_t min;
    uint32_t ses;

    started_time_t started_time;
    time_now_t time_now;

    uint32_t target_end_sec; /* absolute monotonic time at which the current phase (study/break) must end */

    uint8_t validator_status;

} timer_config_t;

void vext_controller(timer_config_t *timer_config);

#endif