#include <stdio.h>
#include <string.h>

#include "../../sounds/tone.h"
#include "../input/input.h"
#include "alarm.h"

typedef void (*alarm_player_t)(void);

typedef struct
{
    const char *name;
    alarm_player_t play;
} alarm_entry_t;

static const alarm_entry_t g_alarms[] = {
    {"megalovania", play_megalovania},
    {"remix", play_megalovania_remix},
    {"jazz", play_megalovania_jazz_remix},
    {"call-response", play_megalovania_call_and_response},
    {"pvz", play_megalovania_pvz_style},
    {"tetris", play_tetris},
    {"mario", play_mario_theme},
    {"imperial", play_imperial_march},
    {"addictive", play_addictive_melody},
    {"beep", play_some_beep},
};

#define ALARM_COUNT (sizeof(g_alarms) / sizeof(g_alarms[0]))

static alarm_player_t find_alarm(const char *name)
{
    size_t i;

    if (name == NULL)
        return NULL;

    for (i = 0; i < ALARM_COUNT; i++)
    {
        if (strcmp(g_alarms[i].name, name) == 0)
            return g_alarms[i].play;
    }

    return NULL;
}

int vext_alarm_is_valid(const char *name)
{
    return vext_alarm_is_none(name) || find_alarm(name) != NULL;
}

int vext_alarm_is_none(const char *name)
{
    return name != NULL && strcmp(name, VEXT_ALARM_NONE) == 0;
}

void vext_alarm_print_names(FILE *stream)
{
    size_t i;

    for (i = 0; i < ALARM_COUNT; i++)
    {
        if (i > 0)
            fprintf(stream, ", ");
        fprintf(stream, "%s", g_alarms[i].name);
    }

    fprintf(stream, ", %s", VEXT_ALARM_NONE);
}

void vext_alarm(const char *name)
{
    alarm_player_t play = find_alarm(name);

    if (play == NULL)
        return; /* "none" or an unknown name */

    /*
     * Without an interactive terminal there is no reliable way to dismiss a
     * looping alarm, so fall back to the terminal bell and keep going.
     */
    if (!vext_input_is_interactive())
    {
        printf("\a\n");
        return;
    }

    if (!vext_audio_start())
    {
        printf("\a\nAlarm: audio playback is unavailable (install alsa-utils).\n");
        printf("Press [ENTER] to continue...\n");

        while (!vext_input_interrupted())
        {
            vext_key_t key = vext_input_poll(VEXT_INPUT_WAIT_FOREVER);
            if (key == VEXT_KEY_ENTER || key == VEXT_KEY_QUIT)
                break;
        }

        return;
    }

    stop_playback = 0;
    printf("\nAlarm ringing! Press any key to stop...\n");

    while (!stop_playback &&
           !vext_input_interrupted() &&
           !vext_input_quit_requested())
    {
        play();
    }

    if (vext_input_interrupted() || vext_input_quit_requested())
        stop_playback = 1;
}
