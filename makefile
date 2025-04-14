CC = gcc
CFLAGS =-Wall -Wextra -Werror -std=c99
TARGET = CComp
SRC_DIR = src
OBJ_DIR = obj
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

.PHONY: all clean

all: $(TARGET)

# Création du répertoire bin avant la compilation
$(OBJ_DIR):
	mkdir -p $@

# Règle de compilation avec dépendance sur le répertoire bin
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Édition de liens
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@


clean:
	rm -rf *.S $(OBJ_DIR) $(TARGET) a.out   

