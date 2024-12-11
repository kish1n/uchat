#include "config.h"
#include <stdio.h>
#include <string.h>
#include <yaml.h>

int load_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening config file");
        return -1;
    }

    yaml_parser_t parser;
    yaml_token_t token;

    if (!yaml_parser_initialize(&parser)) {
        fputs("Error initializing YAML parser\n", stderr);
        fclose(file);
        return -1;
    }

    yaml_parser_set_input_file(&parser, file);
    char current_key[64] = {0};
    char section[16] = {0};

    while (1) {
        yaml_parser_scan(&parser, &token);
        if (token.type == YAML_STREAM_END_TOKEN) {
            break;
        }

        if (token.type == YAML_KEY_TOKEN) {
            yaml_parser_scan(&parser, &token);
            if (token.type == YAML_SCALAR_TOKEN) {
                strncpy(current_key, (char *)token.data.scalar.value, sizeof(current_key) - 1);
                current_key[sizeof(current_key) - 1] = '\0';

                if (strcmp(current_key, "database") == 0 || strcmp(current_key, "server") == 0 ||
                    strcmp(current_key, "logging") == 0 || strcmp(current_key, "security") == 0) {
                    strncpy(section, current_key, sizeof(section) - 1);
                    section[sizeof(section) - 1] = '\0';
                }
            }
        } else if (token.type == YAML_VALUE_TOKEN) {
            yaml_parser_scan(&parser, &token);
            if (token.type == YAML_SCALAR_TOKEN) {
                if (strcmp(section, "database") == 0) {
                    if (strcmp(current_key, "url") == 0) {
                        strncpy(config->database.url, (char *)token.data.scalar.value, sizeof(config->database.url) - 1);
                        config->database.url[sizeof(config->database.url) - 1] = '\0';
                    } else if (strcmp(current_key, "max_connections") == 0) {
                        config->database.max_connections = atoi((char *)token.data.scalar.value);
                    }
                } else if (strcmp(section, "server") == 0) {
                    if (strcmp(current_key, "port") == 0) {
                        strncpy(config->server.port, (char *)token.data.scalar.value, sizeof(config->server.port) - 1);
                        config->server.port[sizeof(config->server.port) - 1] = '\0';
                    } else if (strcmp(current_key, "base_path") == 0) {
                        strncpy(config->server.base_path, (char *)token.data.scalar.value, sizeof(config->server.base_path) - 1);
                        config->server.base_path[sizeof(config->server.base_path) - 1] = '\0';
                    } else if (strcmp(current_key, "timeout") == 0) {
                        config->server.timeout = atoi((char *)token.data.scalar.value);
                    } else if (strcmp(current_key, "max_clients") == 0) {
                        config->server.max_clients = atoi((char *)token.data.scalar.value);
                    }
                } else if (strcmp(section, "logging") == 0) {
                    if (strcmp(current_key, "level") == 0) {
                        strncpy(config->logging.level, (char *)token.data.scalar.value, sizeof(config->logging.level) - 1);
                        config->logging.level[sizeof(config->logging.level) - 1] = '\0';
                    } else if (strcmp(current_key, "format") == 0) {
                        strncpy(config->logging.format, (char *)token.data.scalar.value, sizeof(config->logging.format) - 1);
                        config->logging.format[sizeof(config->logging.format) - 1] = '\0';
                    } else if (strcmp(current_key, "output_file") == 0) {
                        strncpy(config->logging.output_file, (char *)token.data.scalar.value, sizeof(config->logging.output_file) - 1);
                        config->logging.output_file[sizeof(config->logging.output_file) - 1] = '\0';
                    }
                } else if (strcmp(section, "security") == 0) {
                    if (strcmp(current_key, "jwt_secret") == 0) {
                        strncpy(config->security.jwt_secret, (char *)token.data.scalar.value, sizeof(config->security.jwt_secret) - 1);
                        config->security.jwt_secret[sizeof(config->security.jwt_secret) - 1] = '\0';
                    }
                    if (strcmp(current_key, "message_secret") == 0) {
                        strncpy(config->security.message_secret, (char *)token.data.scalar.value, sizeof(config->security.message_secret) - 1);
                        config->security.message_secret[sizeof(config->security.message_secret) - 1] = '\0';
                    }
                }
            }
        }

        yaml_token_delete(&token);
    }

    yaml_parser_delete(&parser);
    fclose(file);
    return 0;
}
