CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -I.
LDFLAGS = -lm

SRC = \
src/main.c \
src/cli/cli.c \
src/timer/counter.c \
src/timer/timer.c \
src/timer/print.c \
src/timer/alarm.c \
sounds/tone.c

TARGET = vex-timer

BUILD_DIR = build
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
DEP = $(OBJ:.o=.d)

# ---- ألوان للإخراج (تُعطَّل تلقائياً إذا لم يكن الطرفية تدعم الألوان) ----
BLUE   = \033[1;34m
GREEN  = \033[1;32m
YELLOW = \033[1;33m
RED    = \033[1;31m
RESET  = \033[0m

.PHONY: all clean rebuild

all: $(TARGET)

$(TARGET): $(OBJ)
	@printf "$(GREEN)==> Linking$(RESET)  %s\n" "$@"
	@$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@printf "$(GREEN)==> Build finished:$(RESET) ./%s\n" "$(TARGET)"

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@printf "$(BLUE)==> Compiling$(RESET) %s\n" "$<"
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEP)

clean:
	@printf "$(YELLOW)==> Cleaning build artifacts$(RESET)\n"
	@rm -rf $(BUILD_DIR) $(TARGET)

rebuild: clean all