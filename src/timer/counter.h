#ifndef COUNTER_H
#define COUNTER_H

#include <stdint.h>

#include "timer.h"

uint64_t vext_now_ns(void);
uint_fast8_t vext_counter_v2(timer_config_t *timer_config);

#endif
