#ifndef VEXT_INPUT_H
#define VEXT_INPUT_H

#include <stdint.h>

typedef enum
{
    VEXT_KEY_NONE = 0,
    VEXT_KEY_ENTER,
    VEXT_KEY_PAUSE,
    VEXT_KEY_SKIP,
    VEXT_KEY_QUIT,
    VEXT_KEY_OTHER
} vext_key_t;

#define VEXT_INPUT_WAIT_FOREVER UINT64_MAX

int vext_input_init(void);
void vext_input_restore(void);
int vext_input_is_interactive(void);
vext_key_t vext_input_poll(uint64_t timeout_ns);
int vext_input_interrupted(void);
int vext_input_interrupt_signal(void);
int vext_input_quit_requested(void);

#endif
