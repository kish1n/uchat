#include "config.h"
#include <stdio.h>
#include <string.h>
#include <yaml.h>

int load_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Ошибка открытия файла конфигурации");
        return -1;
    }

    yaml_parser_t parser;
    yaml_token_t token;

    if (!yaml_parser_initialize(&parser)) {
        fputs("Ошибка инициализации парсера YAML\n", stderr);
        fclose(file);
        return -1;
    }

    yaml_parser_set_input_file(&parser, file);

    // Переменные для отслеживания текущих ключей
    char current_key[64] = {0};
    int in_database = 0, in_server = 0;

    // Чтение токенов YAML
    while (1) {
        yaml_parser_scan(&parser, &token);
        if (token.type == YAML_STREAM_END_TOKEN) {
            break;
        }

        if (token.type == YAML_KEY_TOKEN) {
            // Чтение ключа
            yaml_parser_scan(&parser, &token);
            if (token.type == YAML_SCALAR_TOKEN) {
                strncpy(current_key, (char *)token.data.scalar.value, sizeof(current_key) - 1);
                current_key[sizeof(current_key) - 1] = '\0';

                // Определение текущего раздела
                if (strcmp(current_key, "database") == 0) {
                    in_database = 1;
                    in_server = 0;
                } else if (strcmp(current_key, "server") == 0) {
                    in_database = 0;
                    in_server = 1;
                }
            }
        } else if (token.type == YAML_VALUE_TOKEN) {
            // Чтение значения
            yaml_parser_scan(&parser, &token);
            if (token.type == YAML_SCALAR_TOKEN) {
                if (in_database && strcmp(current_key, "url") == 0) {
                    strncpy(config->database.url, (char *)token.data.scalar.value, sizeof(config->database.url) - 1);
                    config->database.url[sizeof(config->database.url) - 1] = '\0';
                } else if (in_server && strcmp(current_key, "port") == 0) {
                    strncpy(config->server.port, (char *)token.data.scalar.value, sizeof(config->server.port) - 1);
                    config->server.port[sizeof(config->server.port) - 1] = '\0';
                } else if (in_server && strcmp(current_key, "base_path") == 0) {
                    strncpy(config->server.base_path, (char *)token.data.scalar.value, sizeof(config->server.base_path) - 1);
                    config->server.base_path[sizeof(config->server.base_path) - 1] = '\0';
                }
            }
        }

        yaml_token_delete(&token);
    }

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    fclose(file);
    return 0;
}
