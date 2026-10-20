# Vex CLI Study Timer ⏱

<p align="center">
  <img src="assets/demo.png" alt="Vex CLI Study Timer Demo" width="700">
</p>

<p align="center">
  <a href="https://github.com/E-Vex/vex-cli-study-timer/releases/latest">
    <img src="https://img.shields.io/github/v/release/E-Vex/vex-cli-study-timer?color=yellow" alt="Latest release">
  </a>
  <a href="LICENSE">
    <img src="https://img.shields.io/github/license/E-Vex/vex-cli-study-timer?color=yellow" alt="License">
  </a>
</p>

A small command-line study timer written in C for Linux. It schedules focus
sessions and breaks, draws a live progress display, and plays a synthesized
chiptune alarm.

The timer uses raw Linux `clock_gettime(CLOCK_MONOTONIC)` and `nanosleep`
system calls through inline x86_64 assembly. Each phase is driven by an
absolute monotonic deadline, so delayed renders or interrupted sleeps do not
make the countdown drift.

## Features

- Configurable study sessions, focus duration, and break duration
- Live countdown, progress bar, and session counter
- Drift-resistant monotonic timing
- Built-in synthesized alarm music
- Strict CLI parsing with clear error messages and exit codes
- Terminal-bell fallback when `aplay` is unavailable

## Requirements

- Linux on **x86_64** (the raw syscall module is architecture-specific)
- GCC and `make`
- `aplay` from `alsa-utils` for sound (optional)
- A terminal that supports ANSI escape sequences

## Build

```bash
make
```

Run `make clean` to remove build artifacts, or `make rebuild` to start fresh.

You can also compile manually:

```bash
gcc -Wall -Wextra -std=c11 -O2 -I. \
  src/main.c src/cli/cli.c src/timer/counter.c src/timer/timer.c \
  src/timer/print.c src/timer/alarm.c sounds/tone.c \
  -lm -o vex-timer
```

## Usage

```bash
./vex-timer -s 4 -t 25 -b 5
```

All three options are required:

| Option | Long form | Description | Range |
| --- | --- | --- | --- |
| `-s` | `--sessions` | Number of study sessions | 1-255 |
| `-t` | `--focus` | Focus duration in minutes | 1-255 |
| `-b` | `--break` | Break duration in minutes | 1-255 |
| `-h` | `--help` | Show help and exit | - |
| `-v` | `--version` | Show version and exit | - |

Example configurations:

```bash
# Four 25-minute focus sessions with 5-minute breaks
./vex-timer -s 4 -t 25 -b 5

# Same schedule using long options
./vex-timer --sessions 4 --focus 25 --break 5

# A single 50-minute focus session
./vex-timer -s 1 -t 50 -b 1
```

For the first example, the timer shows:

```text
Sessions:  4
Study   :  25 minutes
Break   :  5 minutes
Total   :  115 minutes
```

`Total` counts the three breaks between the four sessions, not a break after
the final session.

## Controls

- **Enter** stops the alarm when a phase ends.
- **Ctrl+C** exits the program.
- If `aplay` is missing, the alarm uses the terminal bell and waits for Enter.

## How It Works

1. `src/cli/cli.c` validates the command-line arguments and builds a
   `timer_config_t`.
2. Each focus or break phase stores an absolute deadline using
   `CLOCK_MONOTONIC` nanoseconds.
3. `vext_counter_v2()` calculates the remaining time from the deadline,
   updates the display once per second, and resumes `nanosleep` if it is
   interrupted.
4. `src/timer/timer.c` runs the study/break state machine until all sessions
   are complete.
5. `sounds/tone.c` synthesizes the alarm samples and pipes them to `aplay`.

## Project Structure

```text
vex-cli-study-timer/
├── assets/
│   └── demo.png
├── sounds/
│   ├── tone.c
│   └── tone.h
├── src/
│   ├── main.c
│   ├── version.h
│   ├── cli/
│   │   ├── cli.c
│   │   └── cli.h
│   └── timer/
│       ├── alarm.c
│       ├── alarm.h
│       ├── counter.c
│       ├── counter.h
│       ├── internals/
│       │   ├── nanosleep_syscall_diagram.txt
│       │   └── syscall_execution_flow.txt
│       ├── print.c
│       ├── print.h
│       ├── timer.c
│       └── timer.h
├── CHANGELOG.md
├── LICENSE
├── Makefile
└── README.md
```

## Exit Codes

| Code | Meaning |
| --- | --- |
| `0` | Completed successfully, or `--help` / `--version` was used |
| `1` | Invalid arguments or configuration |
| `130` | Interrupted with Ctrl+C (standard shell signal status) |

## Notes

- This release keeps the original interaction model: no pause, skip, or quit
  keys beyond Ctrl+C.
- The timer accepts whole minutes only.
- The direct syscall implementation intentionally targets Linux x86_64 and
  will not build on other operating systems or architectures.

## License

MIT. See [LICENSE](LICENSE).

Built by [E-Vex](https://github.com/E-Vex).
