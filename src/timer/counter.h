#ifndef COUNTER_H
#define COUNTER_H

#include <stdint.h>

#include "timer.h"

typedef enum
{
    VEXT_COUNTER_DONE    = 0x0,  /* the phase deadline has been reached   */
    VEXT_COUNTER_SKIPPED = 0x1,  /* the user skipped the rest of the phase */
    VEXT_COUNTER_QUIT    = 0x2,  /* the user (or a signal) asked to quit   */
    VEXT_COUNTER_RUNNING = 0xFF  /* the phase is still in progress         */
} vext_counter_status_t;

uint64_t vext_now_ns(void);
vext_counter_status_t vext_counter_v2(timer_config_t *timer_config);

#endif
