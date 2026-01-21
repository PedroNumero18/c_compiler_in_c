SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = Include

CC = gcc
CFLAGS =-Wall -Wextra -Werror -std=c99 -I$(INCLUDE_DIR)
TARGET = CComp
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

.PHONY: all clean debug distclean

all: $(TARGET)

debug: CFLAGS += -g
debug: $(TARGET)

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

distclean:
	rm -rf *.S $(OBJ_DIR) $(TARGET) a.out  *.s *.asm

clean:
	rm -rf *.S $(TARGET) a.out  

