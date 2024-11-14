#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char url[256];       // URL базы данных
} DatabaseConfig;

typedef struct {
    char port[16];       // Порт сервера
    char base_path[64];  // Базовый путь API
} ServerConfig;

typedef struct {
    DatabaseConfig database;
    ServerConfig server;
} Config;

// Функция для загрузки конфигурации
int load_config(const char *filename, Config *config);

#endif // CONFIG_H
