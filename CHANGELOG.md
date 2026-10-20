# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

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
