# Имя итогового исполняемого файла
TARGET = uchat

# Компилятор
CC = gcc

# Флаги компиляции
#CFLAGS = -Wall -Wextra -std=gnu11 -g -I/usr/include/postgresql
CFLAGS = -Wall -Wextra -std=gnu11 -g -I/usr/local/include/postgresql@14 -I/usr/local/Cellar/libpq/17.2/include

# Путь до всех исходников
SRC_DIR = src

# Путь до всех объектов
OBJ_DIR = build

# Найти все файлы с исходным кодом + main.c
SRC_FILES = $(shell find $(SRC_DIR) -type f -name "*.c")
OBJ_FILES = $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(SRC_FILES:.c=.o))

# Линковка библиотек
#LIBS = -lpq -lyaml -lmicrohttpd -ljson-c -lcrypto -ljwt -lsodium -lwebsockets -lsqlite3 -luuid
LIBS = -L/usr/local/lib -L/usr/local/Cellar/libpq/17.2/lib -lpq -lyaml -lmicrohttpd -ljson-c -lcrypto -ljwt -lsodium -lsqlite3 -lwebsockets

# Правило для компиляции всех объектов
all: $(TARGET)

# Компиляция исполняемого файла
$(TARGET): $(OBJ_FILES)
	@echo "Linking..."
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "Build complete: $(TARGET)"

# Компиляция объектных файлов
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Компиляция main.c
$(OBJ_DIR)/main.o: main.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c main.c -o $(OBJ_DIR)/main.o

# Очистка
clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR) $(TARGET) main.o
	@echo "Clean complete."

# Файлы и директории для управления Makefile
.PHONY: all clean
