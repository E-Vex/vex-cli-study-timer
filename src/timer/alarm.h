#ifndef ALARM_H
#define ALARM_H

#include <stdio.h>

#define VEXT_DEFAULT_ALARM "megalovania"
#define VEXT_ALARM_NONE "none"

int vext_alarm_is_valid(const char *name);
int vext_alarm_is_none(const char *name);
void vext_alarm_print_names(FILE *stream);
void vext_alarm(const char *name);

#endif
