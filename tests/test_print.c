#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../src/timer/print.h"
#include "../src/timer/timer.h"
#include "test_util.h"

/* Every SGR escape the live display can emit. */
static const char *const g_color_escapes[] = {
    "\033[0m", "\033[1m", "\033[2m", "\033[92m",
    "\033[93m", "\033[95m", "\033[96m",
};

static int has_color_escape(const char *text)
{
    size_t i;

    for (i = 0; i < sizeof(g_color_escapes) / sizeof(g_color_escapes[0]); i++)
    {
        if (strstr(text, g_color_escapes[i]) != NULL)
            return 1;
    }

    return 0;
}

/*
 * Runs func(config) with stdout redirected into a pipe and copies what it
 * printed into buf (up to size - 1 bytes, NUL-terminated).
 */
static size_t capture_stdout(char *buf, size_t size,
                             void (*func)(const timer_config_t *),
                             const timer_config_t *config)
{
    int pipe_fds[2];
    int saved_stdout;
    size_t total = 0;
    ssize_t count;

    fflush(stdout);

    if (pipe(pipe_fds) != 0)
        return 0;

    saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout < 0)
    {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return 0;
    }

    dup2(pipe_fds[1], STDOUT_FILENO);
    close(pipe_fds[1]);

    func(config);
    fflush(stdout);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    while (total + 1 < size &&
           (count = read(pipe_fds[0], buf + total, size - 1 - total)) > 0)
        total += (size_t)count;

    close(pipe_fds[0]);
    buf[total] = '\0';

    return total;
}

static void test_color_toggle(void)
{
    vext_color_set_enabled(1);
    CHECK(vext_color_enabled() == 1);

    vext_color_set_enabled(0);
    CHECK(vext_color_enabled() == 0);

    vext_color_set_enabled(42); /* any non-zero value enables */
    CHECK(vext_color_enabled() == 1);

    vext_color_set_enabled(1);
}

static void test_break_frame_colors(void)
{
    timer_config_t config = {0};
    char buf[512];
    size_t len;

    config.session = 4;
    config.session_time = 25;
    config.break_time = 5;
    config.ses = 2;
    config.sec = 12;
    config.min = 3;
    config.validator_status = VEXT_STATUS_BREAK_PHASE;

    vext_color_set_enabled(1);
    len = capture_stdout(buf, sizeof(buf), vext_printer, &config);
    CHECK(len > 0);
    CHECK(strstr(buf, "Break") != NULL);
    CHECK(strstr(buf, "\033[96m") != NULL); /* break color is used */

    vext_color_set_enabled(0);
    len = capture_stdout(buf, sizeof(buf), vext_printer, &config);
    CHECK(len > 0);
    CHECK(strstr(buf, "Break") != NULL);
    CHECK(!has_color_escape(buf)); /* plain frames carry no color codes */

    vext_color_set_enabled(1);
}

static void test_finish_frame_colors(void)
{
    timer_config_t config = {0};
    char buf[2048];
    size_t len;

    config.session = 4;
    config.session_time = 25;
    config.break_time = 5;
    config.ses = 4;
    config.validator_status = VEXT_STATUS_FINISH;

    vext_color_set_enabled(1);
    len = capture_stdout(buf, sizeof(buf), vext_printer, &config);
    CHECK(len > 0);
    CHECK(strstr(buf, "Well done") != NULL);
    CHECK(strstr(buf, "\033[93m") != NULL); /* banner/finish color is used */

    vext_color_set_enabled(0);
    len = capture_stdout(buf, sizeof(buf), vext_printer, &config);
    CHECK(len > 0);
    CHECK(strstr(buf, "Well done") != NULL);
    CHECK(!has_color_escape(buf));

    vext_color_set_enabled(1);
}

void test_print(void)
{
    test_color_toggle();
    test_break_frame_colors();
    test_finish_frame_colors();
}
