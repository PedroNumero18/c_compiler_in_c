CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -std=c99
TARGET = CComp
SRC_DIR = src
BIN_DIR = bin
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC))

.PHONY: all clean

all: $(TARGET)

# Création du répertoire bin avant la compilation
$(BIN_DIR):
	mkdir -p $@

# Règle de compilation avec dépendance sur le répertoire bin
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Édition de liens
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@


clean:
	rm -rf $(BIN_DIR) $(TARGET)

