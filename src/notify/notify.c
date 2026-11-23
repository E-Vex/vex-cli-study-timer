#include <stdio.h>
#include <stdlib.h>

#include "notify.h"

static int g_notify_checked = 0;
static int g_notify_available = 0;
static int g_notify_enabled = 1;

int vext_notify_available(void)
{
    if (!g_notify_checked)
    {
        g_notify_checked = 1;
        g_notify_available = (system("command -v notify-send >/dev/null 2>&1") == 0);
    }

    return g_notify_available;
}

void vext_notify_set_enabled(int enabled)
{
    g_notify_enabled = enabled ? 1 : 0;
}

int vext_notify_enabled(void)
{
    return g_notify_enabled;
}

void vext_notify(const char *summary, const char *body)
{
    char command[512];
    int written;

    if (!g_notify_enabled || summary == NULL || body == NULL)
        return;

    if (!vext_notify_available())
        return;

    /*
     * Summary and body are internal, quote-free strings, so plain single
     * quoting is safe. A truncated command would be malformed; skip it.
     */
    written = snprintf(command, sizeof(command),
                       "notify-send -a vex-timer -u normal '%s' '%s'", summary, body);

    if (written > 0 && written < (int)sizeof(command))
        system(command);
}
