#ifndef CONFIG_H
#define CONFIG_H

#include <sqlite3.h>
#include "../http_response/response.h"
#include "../httputils/httpcontext.h"

typedef enum {
    DEBUG,
    INFO,
    WARN,
    ERROR
} LogLevel;

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
    char level[16];
    char format[16];
    char output_file[256];
} LoggingConfig;

typedef struct {
    char jwt_secret[256];
} SecurityConfig;

typedef struct {
    DatabaseConfig database;
    ServerConfig server;
    LoggingConfig logging;
    SecurityConfig security;
} Config;

int load_config(const char *filename, Config *config);
void init_logger(const LoggingConfig *logging_config);
void logging(LogLevel level, const char *format, ...);
void close_logger();

#endif // CONFIG_H
