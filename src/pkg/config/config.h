#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char url[256];              // URL подключения к базе данных
    int max_connections;        // Максимальное количество подключений
} DatabaseConfig;

typedef struct {
    char port[16];              // Порт для HTTP-сервера
    char base_path[64];         // Базовый путь для API
    int timeout;                // Таймаут запросов в секундах
    int max_clients;            // Максимальное количество одновременных клиентов
} ServerConfig;

typedef struct {
    char level[16];             // Уровень логирования
    char format[16];            // Формат логирования ("text" или "json")
    char output_file[256];      // Файл для логирования
} LoggingConfig;

typedef struct {
    char jwt_secret[256];       // Секрет для JWT-токенов
} SecurityConfig;

typedef struct {
    DatabaseConfig database;
    ServerConfig server;
    LoggingConfig logging;
    SecurityConfig security;
} Config;

int load_config(const char *filename, Config *config);

#endif // CONFIG_H
