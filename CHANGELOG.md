# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added

- 5 new synthesized chiptune alarm options: `hard-to-say-goodbye`, `cinematic`, 
  `garden-guardian`, `final-showdown`, and `victory`.

## [1.1.0] - 2026-10-21

### Added

- Interactive controls during focus and break phases:
  - `Space` or `P` pauses and resumes the current phase.
  - `S` skips the current phase.
  - `Q` quits immediately.
- `-a`/`--alarm` option for choosing the phase alarm, including `none`.
- Alarm catalogue with ten synthesized chiptune options.
- Unit tests for CLI parsing and timer validation, run with `make test`.
- GitHub Actions CI for Linux with GCC and Clang, and macOS with Clang.

### Changed

- Replaced the x86_64 inline-assembly syscall layer with portable POSIX
  `clock_gettime`/`nanosleep` calls, so the timer builds on other POSIX systems
  and architectures.
- Alarm and finish-music playback now reacts to key presses during rests and
  long notes instead of waiting for the current note to end.
- The finish fanfare is skipped when `--alarm none` is selected.
- The README now documents controls, alarm names, testing, and the portable
  timing model.

### Fixed

- Terminal settings are restored when the program exits or receives a handled
  signal.
- Non-interactive stdin no longer makes alarms stop immediately.
- Pause/resume preserves sub-second time instead of rounding it away.
- Strict integer parsing now rejects leading signs and whitespace.

[1.1.0]: https://github.com/E-Vex/vex-cli-study-timer/releases/tag/v1.1.0

## [1.0.0] - 2026-10-21

First tagged release.

### Added

- `-h`/`--help` and `-v`/`--version` options.
- Long option aliases: `--sessions`, `--focus`, and `--break`.
- Version header (`src/version.h`).
- Terminal-bell fallback when `aplay` is unavailable.

### Changed

- The README now documents the real project structure, requirements, options,
  controls, exit codes, and timing model.
- Release builds use `-O2`.

### Fixed

- Duplicate, missing, and unknown command-line options are now rejected.
- Integer parsing is strict and rejects zero, overflow, and trailing garbage.
- Invalid arguments and out-of-range values now exit with a failure status.
- Timing now uses nanosecond `CLOCK_MONOTONIC` deadlines instead of
  whole-second arithmetic.
- Interrupted `nanosleep` calls are resumed instead of being ignored.
- The finish summary now reports focus, break, and total time correctly,
  without counting a break after the final session.
- The finish box is aligned correctly; its content no longer uses byte-counted
  UTF-8 glyphs.
- Audio failures no longer result in a silent alarm.
- The audio pipe is closed automatically when the program exits.

[1.0.0]: https://github.com/E-Vex/vex-cli-study-timer/releases/tag/v1.0.0
