CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -I. -O2
LDFLAGS = -lm

SRC = \
src/main.c \
src/cli/cli.c \
src/input/input.c \
src/timer/counter.c \
src/timer/timer.c \
src/timer/print.c \
src/timer/alarm.c \
sounds/tone.c

LIB_SRC = $(filter-out src/main.c,$(SRC))

TEST_SRC = \
tests/test_main.c \
tests/test_cli.c \
tests/test_timer.c

TARGET = vex-timer

BUILD_DIR = build
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
DEP = $(OBJ:.o=.d)

TEST_OBJ = $(TEST_SRC:%.c=$(BUILD_DIR)/%.o) $(LIB_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_TARGET = $(BUILD_DIR)/tests/run-tests

# ---- ألوان للإخراج (تُعطَّل تلقائياً إذا لم يكن الطرفية تدعم الألوان) ----
BLUE   = \033[1;34m
GREEN  = \033[1;32m
YELLOW = \033[1;33m
RED    = \033[1;31m
RESET  = \033[0m

.PHONY: all clean rebuild test

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
