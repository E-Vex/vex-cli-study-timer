#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <time.h>

#include "counter.h"

/*
 * =====================================================================================
 * PLATFORM DEPENDENCY WARNING:
 * The functions below rely on direct Linux system calls using inline x86_64 assembly
 * (rax, rdi, rsi registers and the 'syscall' instruction).
 *
 * Target Environment:
 *   - OS: Linux ONLY
 *   - Architecture: x86_64 (AMD64) ONLY
 *
 * Non-Portable: This code will fail to compile or run on Windows, macOS,
 * or non-x86_64 architectures (ARM/AArch64, x86_32, RISC-V).
 * =====================================================================================
 */

#if !defined(__linux__) || !defined(__x86_64__)
#error "This module uses raw x86_64 Linux system calls and can only be compiled on Linux x86_64."
#endif

typedef struct
{
    long seconds;
    long nanoseconds;
} vex_timespec_t;

static long vext_syscall_clock_gettime(vex_timespec_t *time_spec)
{
    long result;

    __asm__ volatile(
        "syscall"
        : "=a"(result)
        : "a"((long)SYS_clock_gettime), "D"((long)CLOCK_MONOTONIC), "S"(time_spec)
        : "rcx", "r11", "memory");

    return result;
}

static long vext_syscall_nanosleep(const vex_timespec_t *request, vex_timespec_t *remaining)
{
    long result;

    __asm__ volatile(
        "syscall"
        : "=a"(result)
        : "a"((long)SYS_nanosleep), "D"(request), "S"(remaining)
        : "rcx", "r11", "memory");

    return result;
}

uint64_t vext_now_ns(void)
{
    vex_timespec_t time_spec = {0, 0};

    if (vext_syscall_clock_gettime(&time_spec) != 0)
        return 0;

    return (uint64_t)time_spec.seconds * VEXT_NS_PER_SECOND +
           (uint64_t)time_spec.nanoseconds;
}

static void vext_sleep_ns(uint64_t duration_ns)
{
    uint64_t remaining_ns = duration_ns;

    while (remaining_ns > 0)
    {
        vex_timespec_t request = {
            (long)(remaining_ns / VEXT_NS_PER_SECOND),
            (long)(remaining_ns % VEXT_NS_PER_SECOND)};
        vex_timespec_t remaining = {0, 0};
        long result = vext_syscall_nanosleep(&request, &remaining);

        if (result == 0)
            return;

        if (result != -EINTR)
            return;

        remaining_ns = (uint64_t)remaining.seconds * VEXT_NS_PER_SECOND +
                       (uint64_t)remaining.nanoseconds;
    }
}

/*
 * The countdown is based on an absolute CLOCK_MONOTONIC deadline rather than on
 * accumulating sleeps. This keeps it accurate even when a tick is delayed by
 * terminal output, alarm handling, or an interrupted nanosleep().
 */
uint_fast8_t vext_counter_v2(timer_config_t *timer_config)
{
    uint64_t now_ns = vext_now_ns();
    uint64_t remaining_ns = 0;
    uint64_t remaining_seconds;
    uint64_t sleep_ns;

    if (timer_config->target_end_ns > now_ns)
        remaining_ns = timer_config->target_end_ns - now_ns;

    /* Round up so a fresh 25-minute phase starts by displaying 25:00. */
    remaining_seconds = (remaining_ns + VEXT_NS_PER_SECOND - 1) / VEXT_NS_PER_SECOND;
    timer_config->min = (uint32_t)(remaining_seconds / 60);
    timer_config->sec = (uint32_t)(remaining_seconds % 60);

    if (remaining_ns == 0)
        return 0x0; /* End of the timer */

    /* Wake up when the displayed second changes. */
    sleep_ns = remaining_ns % VEXT_NS_PER_SECOND;
    if (sleep_ns == 0)
        sleep_ns = VEXT_NS_PER_SECOND;

    vext_sleep_ns(sleep_ns);
    return 0xFF; /* Time is working */
}
