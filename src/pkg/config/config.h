#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char url[256];
} DatabaseConfig;

typedef struct {
    char port[16];
    char base_path[64];
} ServerConfig;

typedef struct {
    char level[16];
    char format[16];
} LoggingConfig;

typedef struct {
    DatabaseConfig database;
    ServerConfig server;
    LoggingConfig logging;
} Config;

int load_config(const char *filename, Config *config);

#endif // CONFIG_H
