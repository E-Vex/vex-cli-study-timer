#include <stdio.h>
#include <string.h>

#include "timer.h"

/*-----------------------------------------------------------------------------------------------------------------------------*/
#define BOLD "\033[1m"
#define DIM "\033[2m"
#define BRIGHT_YELLOW "\033[93m"
#define BRIGHT_GREEN "\033[92m"
#define BRIGHT_CYAN "\033[96m"
#define RESET "\033[0m"

#define STUDY_COLOR BRIGHT_GREEN
#define BREAK_COLOR BRIGHT_CYAN
#define FINISH_COLOR BRIGHT_YELLOW

#define LOGO_LINE(txt) BRIGHT_YELLOW BOLD txt RESET "\n"

#define BAR_WIDTH 20

#define BOX_INNER_WIDTH 65

static void print_box_border(const char *left_corner, const char *right_corner)
{
    int i;

    printf("%s", left_corner);
    for (i = 0; i < BOX_INNER_WIDTH; i++)
        printf("═");
    printf("%s\n", right_corner);
}

static void print_box_line(const char *text)
{
    int len = (int)strlen(text);
    int pad = BOX_INNER_WIDTH - len;

    if (pad < 0)
        pad = 0;

    printf("║%s%*s║\n", text, pad, "");
}

static void print_box_centered(const char *text)
{
    int len = (int)strlen(text);
    int pad = BOX_INNER_WIDTH - len;
    int left = pad / 2;
    int right = pad - left;

    if (left < 0)
        left = 0;
    if (right < 0)
        right = 0;

    printf("║%*s%s%*s║\n", left, "", text, right, "");
}

void display_info(const timer_config_t *timer_config)
{
    unsigned int focus_minutes = (unsigned int)timer_config->session * timer_config->session_time;
    unsigned int break_minutes = timer_config->session > 0
                                     ? (unsigned int)(timer_config->session - 1) * timer_config->break_time
                                     : 0;
    unsigned int total_minutes = focus_minutes + break_minutes;

    printf("Sessions:  %u\n", timer_config->session);
    printf("Study   :  %u minutes\n", timer_config->session_time);
    printf("Break   :  %u minutes\n", timer_config->break_time);
    printf("Total   :  %u minutes\n\n", total_minutes);
}

void display_banner(void)
{
    printf(LOGO_LINE("  ██╗   ██╗███████╗██╗  ██╗      ████████╗██╗███╗   ███╗███████╗██████╗ "));
    printf(LOGO_LINE("  ██║   ██║██╔════╝╚██╗██╔╝      ╚══██╔══╝██║████╗ ████║██╔════╝██╔══██╗"));
    printf(LOGO_LINE("  ██║   ██║█████╗   ╚███╔╝ █████╗   ██║   ██║██╔████╔██║█████╗  ██████╔╝"));
    printf(LOGO_LINE("  ╚██╗ ██╔╝██╔══╝   ██╔██╗ ╚════╝   ██║   ██║██║╚██╔╝██║██╔══╝  ██╔══██╗"));
    printf(LOGO_LINE("   ╚████╔╝ ███████╗██╔╝ ██╗         ██║   ██║██║ ╚═╝ ██║███████╗██║  ██║"));
    printf(LOGO_LINE("    ╚═══╝  ╚══════╝╚═╝  ╚═╝         ╚═╝   ╚═╝╚═╝     ╚═╝╚══════╝╚═╝  ╚═╝"));
    printf("\n");
}

void display_finish(const timer_config_t *timer_config)
{
    char sessions_line[80];
    char focus_line[80];
    char break_line[80];
    char total_line[80];
    unsigned int focus_minutes = (unsigned int)timer_config->session * timer_config->session_time;
    unsigned int break_minutes = timer_config->session > 0
                                     ? (unsigned int)(timer_config->session - 1) * timer_config->break_time
                                     : 0;
    unsigned int total_minutes = focus_minutes + break_minutes;

    snprintf(sessions_line, sizeof(sessions_line), "   Sessions completed :  %u", timer_config->session);
    snprintf(focus_line, sizeof(focus_line), "   Focus time         :  %u minutes", focus_minutes);
    snprintf(break_line, sizeof(break_line), "   Break time         :  %u minutes", break_minutes);
    snprintf(total_line, sizeof(total_line), "   Total time         :  %u minutes", total_minutes);

    printf("\n" FINISH_COLOR BOLD);
    print_box_border("╔", "╗");
    print_box_line("");
    print_box_centered("  *  Well done! Session complete  *");
    print_box_line("");
    print_box_line(sessions_line);
    print_box_line(focus_line);
    print_box_line(break_line);
    print_box_line(total_line);
    print_box_line("");
    print_box_border("╚", "╝");
    printf(RESET "\n");
}

static void print_progress_bar(unsigned int remaining_seconds, unsigned int total_seconds, const char *color)
{
    unsigned int elapsed_seconds;
    unsigned int filled;
    unsigned int percent;
    unsigned int i;

    if (total_seconds == 0)
        total_seconds = 1;

    elapsed_seconds = (remaining_seconds >= total_seconds) ? 0 : (total_seconds - remaining_seconds);
    percent = (elapsed_seconds * 100) / total_seconds;
    filled = (elapsed_seconds * BAR_WIDTH) / total_seconds;

    printf("[%s", color);
    for (i = 0; i < filled; i++)
        printf("█");
    printf(RESET DIM);
    for (i = filled; i < BAR_WIDTH; i++)
        printf("░");
    printf(RESET "] %3u%%", percent);
}

void display_study_running(const timer_config_t *timer_config, unsigned int s, unsigned int m, unsigned int h,
                           unsigned int remaining_seconds, unsigned int total_seconds)
{
    printf("\r\033[2K" STUDY_COLOR BOLD "Study" RESET "  %02u:%02u:%02u | session %u/%u  ",
           h, m, s, timer_config->ses, timer_config->session);
    print_progress_bar(remaining_seconds, total_seconds, STUDY_COLOR);
    fflush(stdout);
}

void display_break_running(unsigned int s, unsigned int m, unsigned int h,
                           unsigned int remaining_seconds, unsigned int total_seconds)
{
    printf("\r\033[2K" BREAK_COLOR BOLD "Break" RESET "  %02u:%02u:%02u  ", h, m, s);
    print_progress_bar(remaining_seconds, total_seconds, BREAK_COLOR);
    fflush(stdout);
}

void vext_printer(const timer_config_t *timer_config)
{
    unsigned int s;
    unsigned int m;
    unsigned int h = 0;
    unsigned int remaining_seconds;
    unsigned int total_seconds;

    s = timer_config->sec;
    m = timer_config->min;
    remaining_seconds = (m * 60) + s;

    while (m >= 60)
    {
        h += 1;
        m -= 60;
    }

    if (timer_config->validator_status == VEXT_STATUS_BREAK_PHASE)
    {
        total_seconds = timer_config->break_time * 60;
        display_break_running(s, m, h, remaining_seconds, total_seconds);
    }
    else if (timer_config->validator_status == VEXT_STATUS_FINISH)
    {
        printf("\033[H\033[J"); /* Clear screen */
        display_banner();
        display_finish(timer_config);
    }
    else
    {
        total_seconds = timer_config->session_time * 60;
        display_study_running(timer_config, s, m, h, remaining_seconds, total_seconds);
    }
}
/*-----------------------------------------------------------------------------------------------------------------------------*/
