# Исполняемые файлы
TARGET_SERVER = uchat
TARGET_CLIENT = client_app

# Компилятор
CC = gcc

# Флаги компиляции
CFLAGS = -Wall -Wextra -std=gnu11 -g -I/usr/include/postgresql -I/usr/include/cjson

# GTK-флаги
GTK_FLAGS = `pkg-config --cflags gtk+-3.0`
GTK_LIBS = `pkg-config --libs gtk+-3.0`

# Линковка библиотек
LIBS_SERVER = -lpq -lyaml -lmicrohttpd -ljson-c -lcrypto -ljwt -lsodium -lwebsockets -lsqlite3 -luuid
LIBS_CLIENT = $(GTK_LIBS) -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lcjson

# Путь до исходников
SRC_SERVER = src/server
SRC_CLIENT = src/client

# Путь до объектов
BUILD_SERVER = build/server
BUILD_CLIENT = build/client

# Найти файлы для сервера (включая main.c)
SERVER_SRC = $(shell find $(SRC_SERVER) -type f -name "*.c") $(SRC_SERVER)/main.c
SERVER_OBJ = $(patsubst $(SRC_SERVER)/%.c, $(BUILD_SERVER)/%.o, $(SERVER_SRC))

# Найти файлы для клиента
CLIENT_SRC = $(shell find $(SRC_CLIENT) -type f -name "*.c")
CLIENT_OBJ = $(patsubst $(SRC_CLIENT)/%.c, $(BUILD_CLIENT)/%.o, $(CLIENT_SRC))

# Сборка всех целей
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Сборка сервера
$(TARGET_SERVER): $(SERVER_OBJ)
	@echo "Linking server..."
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS_SERVER)
	@echo "Server build complete: $(TARGET_SERVER)"

# Сборка клиента
$(TARGET_CLIENT): $(CLIENT_OBJ)
	@echo "Linking client..."
	$(CC) $(CFLAGS) $(GTK_FLAGS) $^ -o $@ $(LIBS_CLIENT)
	@echo "Client build complete: $(TARGET_CLIENT)"

# Компиляция объектов сервера
$(BUILD_SERVER)/%.o: $(SRC_SERVER)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Компиляция объектов клиента
$(BUILD_CLIENT)/%.o: $(SRC_CLIENT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(GTK_FLAGS) -c $< -o $@

# Очистка
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_SERVER) $(BUILD_CLIENT) $(TARGET_SERVER) $(TARGET_CLIENT)
	@echo "Clean complete."

# Указание, что цели не являются файлами
.PHONY: all clean
