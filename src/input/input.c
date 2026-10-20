#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "input.h"

static struct termios g_saved_termios;
static int g_initialized = 0;
static int g_interactive = 0;
static volatile sig_atomic_t g_signal = 0;
static volatile sig_atomic_t g_quit_requested = 0;

static void handle_signal(int signal_number)
{
    g_signal = signal_number;
}

static void sleep_ns(uint64_t duration_ns)
{
    struct timespec remaining;

    if (duration_ns == 0 || duration_ns == VEXT_INPUT_WAIT_FOREVER)
        return;

    remaining.tv_sec = (time_t)(duration_ns / 1000000000ULL);
    remaining.tv_nsec = (long)(duration_ns % 1000000000ULL);

    while (nanosleep(&remaining, &remaining) != 0)
    {
        if (errno != EINTR || g_signal != 0)
            break;
    }
}

int vext_input_init(void)
{
    struct sigaction action = {0};
    struct termios raw;

    if (g_initialized)
        return g_interactive;

    g_initialized = 1;
    g_signal = 0;
    g_quit_requested = 0;

    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);

    atexit(vext_input_restore);

    if (!isatty(STDIN_FILENO))
        return 0;

    if (tcgetattr(STDIN_FILENO, &g_saved_termios) != 0)
        return 0;

    raw = g_saved_termios;
    /*
     * Disable canonical input and echo, and handle Ctrl+C ourselves so that
     * Ctrl+Z / Ctrl+\ cannot suspend the process while the terminal is raw.
     */
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO | ISIG);
    raw.c_iflag &= (tcflag_t)~(IXON | ICRNL);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0)
        return 0;

    g_interactive = 1;
    return 1;
}

void vext_input_restore(void)
{
    if (!g_interactive)
        return;

    tcsetattr(STDIN_FILENO, TCSANOW, &g_saved_termios);
    g_interactive = 0;
}

int vext_input_is_interactive(void)
{
    return g_interactive;
}

vext_key_t vext_input_poll(uint64_t timeout_ns)
{
    fd_set read_fds;
    struct timeval timeout;
    struct timeval *timeout_ptr = NULL;
    unsigned char byte;
    ssize_t count;
    int ready;

    if (g_signal != 0)
        return VEXT_KEY_QUIT;

    if (!g_interactive)
    {
        sleep_ns(timeout_ns);
        return g_signal != 0 ? VEXT_KEY_QUIT : VEXT_KEY_NONE;
    }

    if (timeout_ns != VEXT_INPUT_WAIT_FOREVER)
    {
        timeout.tv_sec = (time_t)(timeout_ns / 1000000000ULL);
        timeout.tv_usec = (suseconds_t)((timeout_ns % 1000000000ULL) / 1000ULL);

        /* select() has microsecond resolution; round tiny positive waits up. */
        if (timeout_ns > 0 && timeout.tv_sec == 0 && timeout.tv_usec == 0)
            timeout.tv_usec = 1;

        timeout_ptr = &timeout;
    }

    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    ready = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, timeout_ptr);

    if (ready < 0)
    {
        if (errno == EINTR && g_signal != 0)
            return VEXT_KEY_QUIT;
        return VEXT_KEY_NONE;
    }

    if (ready == 0)
        return VEXT_KEY_NONE;

    count = read(STDIN_FILENO, &byte, 1);

    if (count == 0)
        return VEXT_KEY_QUIT;

    if (count < 0)
    {
        if (errno == EINTR && g_signal != 0)
            return VEXT_KEY_QUIT;
        return VEXT_KEY_NONE;
    }

    switch (byte)
    {
    case 0x03: /* Ctrl+C, since ISIG is disabled in raw mode */
        g_signal = SIGINT;
        return VEXT_KEY_QUIT;

    case '\r':
    case '\n':
        return VEXT_KEY_ENTER;

    case ' ':
    case 'p':
    case 'P':
        return VEXT_KEY_PAUSE;

    case 's':
    case 'S':
        return VEXT_KEY_SKIP;

    case 'q':
    case 'Q':
        g_quit_requested = 1;
        return VEXT_KEY_QUIT;

    default:
        return VEXT_KEY_OTHER;
    }
}

int vext_input_interrupted(void)
{
    return g_signal != 0;
}

int vext_input_interrupt_signal(void)
{
    return (int)g_signal;
}

int vext_input_quit_requested(void)
{
    return g_quit_requested != 0;
}
