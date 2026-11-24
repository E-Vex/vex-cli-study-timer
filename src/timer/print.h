#ifndef PRINT_H
#define PRINT_H

#include "timer.h"

void display_info(const timer_config_t *timer_config);
void display_banner(void);
void display_controls(void);
void display_finish(const timer_config_t *timer_config);
void vext_printer(const timer_config_t *timer_config);

/*
 * Color decisions for the live display. vext_color_init() reads NO_COLOR,
 * TERM, and stdout once at startup; the state can also be forced explicitly
 * (used by the tests).
 */
void vext_color_init(void);
void vext_color_set_enabled(int enabled);
int vext_color_enabled(void);

#endif
