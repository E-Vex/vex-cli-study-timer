#ifndef VEXT_NOTIFY_H
#define VEXT_NOTIFY_H

/*
 * Desktop notifications via notify-send. Notifications are sent only when the
 * helper is installed; otherwise every call is a silent no-op.
 */

int vext_notify_available(void);
void vext_notify_set_enabled(int enabled);
int vext_notify_enabled(void);
void vext_notify(const char *summary, const char *body);

#endif
