#include <stdio.h>
#include <stdlib.h>

#include "../../sounds/tone.h"

extern int stop_playback;

void vext_alarm(void)
{
    if (!vext_audio_start())
    {
        printf("\a\nAlarm: audio playback is unavailable (install alsa-utils).\n");
        printf("Press [ENTER] to continue...\n");

        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;

        return;
    }

    stop_playback = 0;
    printf("\nAlarm ringing! Press [ENTER] to stop...\n");
    play_megalovania();
}
