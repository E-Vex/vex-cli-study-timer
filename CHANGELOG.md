# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [1.4.0] - 2026-11-24

The pending 1.3.1 release was never published, and this cycle ships new
features on top of the fixes below, so it is released as 1.4.0 per
Semantic Versioning.

### Fixed

- The finish fanfare no longer plays through `aplay` when stdin is not a
  terminal or no audio player is installed. Unattended runs now hear a
  single bell, matching the phase alarms and the documented fallback.
- The audio-unavailable prompt now accepts any key, matching the
  documented "any key stops a ringing alarm" behavior. Previously only
  Enter or Q dismissed it.
- While paused, the key poll now waits a steady 200 ms instead of
  shrinking the wait to the remaining phase time, which could busy-poll
  when very little time was left.
- GCC builds with `_FORTIFY_SOURCE` active (Ubuntu enables it by default
  at `-O2`) no longer fail under `-Werror` on the `system()` call that
  sends notifications. The return value is now checked, and a failed
  shell spawn disables notifications for the rest of the run; a nonzero
  exit status from `notify-send` itself is still ignored as transient.

### Changed

- Singular durations now read "1 minute" instead of "1 minutes" in the
  header and the finish summary.

### Added

- `make install` and `make uninstall` targets with `PREFIX` and `DESTDIR`
  support for staged packaging. They now install and remove the
  `vex-timer(1)` manual page alongside the binary.
- A `vex-timer(1)` manual page under `docs/`, installed to
  `$(PREFIX)/share/man/man1` by `make install`.
- CI now also builds and runs the test suite with AddressSanitizer and
  UndefinedBehaviorSanitizer, and smoke-tests `make install` and
  `make uninstall` against a staged `DESTDIR` on every runner.
- More unit tests: counter pause paths, plan-summary edge cases, and
  stricter value-parsing variants.

### Performance

- Alarm samples are written to `aplay` in 8 KB chunks instead of one
  `fwrite` call per sample (44,100 calls per second previously). The
  synthesized byte stream is unchanged; output was verified byte-for-byte
  identical before and after.

## [1.3.0] - 2026-11-23

### Added

- Long break support: `-l`/`--long-break` schedules a longer break and
  `-r`/`--long-break-every` sets the cadence (default 4, requires `-l`).
  Long breaks run between sessions only, never after the final one, and the
  header, live display, and finish summary all account for them.
- Desktop notifications at phase transitions when `notify-send` is installed;
  disable with `--no-notify`. Skipped phases do not notify.
- `--name=value` syntax for long options, e.g. `--focus=25` or `--alarm=tetris`.

### Changed

- Total, focus, and break time are now computed by a shared, unit-tested
  `vext_plan_summary()` helper used by both the header and the finish box.

## [1.2.0] - 2026-10-26

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

[1.3.1]: https://github.com/E-Vex/vex-cli-study-timer/releases/tag/v1.3.1
[1.3.0]: https://github.com/E-Vex/vex-cli-study-timer/releases/tag/v1.3.0
[1.2.0]: https://github.com/E-Vex/vex-cli-study-timer/releases/tag/v1.2.0
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
