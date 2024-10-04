CC = gcc
CFLAGS = -Isrc/include -Wall
SRC_DIR = src
BUILD_DIR = build
OBJ_FILES = $(BUILD_DIR)/main.o $(BUILD_DIR)/network.o $(BUILD_DIR)/handler.o $(BUILD_DIR)/common.o $(BUILD_DIR)/thpool.o
DEPS = $(SRC_DIR)/include/common.h $(SRC_DIR)/network.h $(SRC_DIR)/handler.h $(SRC_DIR)/include/thpool.h
TARGET = server

# Créer le répertoire build si nécessaire
$(shell mkdir -p $(BUILD_DIR))

# Règle par défaut
all: $(TARGET)

# Compilation de l'exécutable
$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) -pthread -o $@ $^

# Compilation des fichiers objet
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/include/%.c  $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Nettoyage du projet
.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)
