CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -I. -O2
LDFLAGS = -lm

SRC = \
src/main.c \
src/cli/cli.c \
src/input/input.c \
src/notify/notify.c \
src/timer/counter.c \
src/timer/timer.c \
src/timer/print.c \
src/timer/alarm.c \
sounds/tone.c

LIB_SRC = $(filter-out src/main.c,$(SRC))

TEST_SRC = \
tests/test_main.c \
tests/test_cli.c \
tests/test_notify.c \
tests/test_print.c \
tests/test_timer.c

TARGET = vex-timer

BUILD_DIR = build
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
DEP = $(OBJ:.o=.d)

TEST_OBJ = $(TEST_SRC:%.c=$(BUILD_DIR)/%.o) $(LIB_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_TARGET = $(BUILD_DIR)/tests/run-tests

# ---- Output colors for build messages ----
BLUE   = \033[1;34m
GREEN  = \033[1;32m
YELLOW = \033[1;33m
RED    = \033[1;31m
RESET  = \033[0m

.PHONY: all clean rebuild test install uninstall

all: $(TARGET)

$(TARGET): $(OBJ)
	@printf "$(GREEN)==> Linking$(RESET)  %s\n" "$@"
	@$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@printf "$(GREEN)==> Build finished:$(RESET) ./%s\n" "$(TARGET)"

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@printf "$(BLUE)==> Compiling$(RESET) %s\n" "$<"
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(TEST_TARGET): $(TEST_OBJ)
	@printf "$(GREEN)==> Linking tests$(RESET) %s\n" "$@"
	@mkdir -p $(dir $@)
	@$(CC) $(TEST_OBJ) -o $@ $(LDFLAGS)

test: $(TEST_TARGET)
	@printf "$(GREEN)==> Running tests$(RESET)\n"
	@$(TEST_TARGET)

-include $(DEP) $(TEST_OBJ:.o=.d)

clean:
	@printf "$(YELLOW)==> Cleaning build artifacts$(RESET)\n"
	@rm -rf $(BUILD_DIR) $(TARGET)

rebuild: clean all

PREFIX ?= /usr/local

install: $(TARGET)
	@printf "$(GREEN)==> Installing$(RESET) %s to %s/bin\n" "$(TARGET)" "$(PREFIX)"
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	@printf "$(GREEN)==> Installing$(RESET) docs/vex-timer.1 to %s/share/man/man1\n" "$(PREFIX)"
	@mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	@cp docs/vex-timer.1 $(DESTDIR)$(PREFIX)/share/man/man1/

uninstall:
	@printf "$(YELLOW)==> Removing$(RESET) %s from %s/bin\n" "$(TARGET)" "$(PREFIX)"
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	@printf "$(YELLOW)==> Removing$(RESET) docs/vex-timer.1 from %s/share/man/man1\n" "$(PREFIX)"
	@rm -f $(DESTDIR)$(PREFIX)/share/man/man1/vex-timer.1
	@# Best-effort cleanup of the directories this package created, like the
	@# uninstall rules autoconf generates: rmdir only succeeds when the
	@# directory is empty, so shared system directories are never touched.
	-@rmdir $(DESTDIR)$(PREFIX)/bin 2>/dev/null || true
	-@rmdir $(DESTDIR)$(PREFIX)/share/man/man1 2>/dev/null || true
	-@rmdir $(DESTDIR)$(PREFIX)/share/man 2>/dev/null || true
	-@rmdir $(DESTDIR)$(PREFIX)/share 2>/dev/null || true
	-@rmdir $(DESTDIR)$(PREFIX) 2>/dev/null || true
	-@if [ -n "$(DESTDIR)" ]; then \
		d="$(DESTDIR)"; p="$(DESTDIR)$(PREFIX)"; \
		while [ "$$p" != "$$d" ] && [ -n "$$p" ] && [ "$$p" != "/" ]; do \
			rmdir "$$p" 2>/dev/null || true; \
			p=$$(dirname "$$p"); \
		done; \
		rmdir "$$d" 2>/dev/null || true; \
	fi
