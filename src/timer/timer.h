#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define VEXT_MAX_SESSIONS 255U
#define VEXT_MAX_MINUTES 255U

#define VEXT_DEFAULT_LONG_BREAK_EVERY 4U

#define VEXT_NS_PER_SECOND 1000000000ULL
#define VEXT_NS_PER_MINUTE (60ULL * VEXT_NS_PER_SECOND)

#define VEXT_EXIT_INTERRUPTED 130

/* Status codes returned by vext_validator / vext_input_validator and the
   internal phase runner. */
typedef enum
{
    VEXT_STATUS_VALID        = 0x0,  /* generic OK / valid state            */
    VEXT_STATUS_FINISH       = 0xA,  /* end of all sessions                 */
    VEXT_STATUS_BREAK_PHASE  = 0xB,  /* end of a study session -> break     */
    VEXT_STATUS_STUDY_DONE   = 0xAA, /* a study session has completed       */
    VEXT_STATUS_BREAK_DONE   = 0xBB, /* a break has completed               */
    VEXT_STATUS_SKIPPED      = 0xCC, /* the current phase was skipped       */
    VEXT_STATUS_QUIT         = 0xDD, /* the user asked to quit              */
    VEXT_STATUS_LONG_BREAK   = 0xBC, /* end of a study session -> long break */
    VEXT_STATUS_OUT_OF_RANGE = 0x1F  /* invalid/out-of-range input          */
} vext_status_t;

typedef struct
{
    uint32_t session;      /* total number of study sessions */
    uint32_t session_time; /* duration of each session, in minutes */
    uint32_t break_time;   /* duration of break between sessions, in minutes */
    uint32_t long_break_time;  /* long break duration in minutes; 0 = disabled */
    uint32_t long_break_every; /* take a long break after this many sessions */
    const char *alarm;     /* selected alarm sound name */

    /*-------------------------------------------------------------------------------*/
    uint32_t sec;
    uint32_t min;
    uint32_t ses;

    uint64_t target_end_ns;        /* absolute monotonic time at which the phase ends */
    uint64_t paused_remaining_ns;  /* frozen time left while paused                   */

    uint8_t paused;
    uint8_t validator_status;

} timer_config_t;

uint_fast8_t vext_validator(timer_config_t *timer_config);
uint_fast8_t vext_input_validator(timer_config_t *timer_config);
int vext_controller(timer_config_t *timer_config);

/* Planned time split of a full run, in minutes. */
typedef struct
{
    unsigned long focus_minutes;      /* time spent focusing */
    unsigned long break_minutes;      /* time spent in short breaks */
    unsigned long long_break_minutes; /* time spent in long breaks */
    unsigned long total_minutes;
} vext_plan_summary_t;

void vext_plan_summary(const timer_config_t *timer_config, vext_plan_summary_t *summary);

/* Returns 1 when the break after completed_sessions sessions is a long one. */
int vext_is_long_break(const timer_config_t *timer_config, uint32_t completed_sessions);

#endif
