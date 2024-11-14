#include <stdio.h>
#include <string.h>
#include <time.h>
#include "logger.h"

static LogLevel current_log_level = INFO;
static int is_json_format = 0;

void init_logger(const Config *config) {
    if (strcmp(config->logging.level, "debug") == 0) {
        current_log_level = DEBUG;
    } else if (strcmp(config->logging.level, "info") == 0) {
        current_log_level = INFO;
    } else if (strcmp(config->logging.level, "warn") == 0) {
        current_log_level = WARN;
    } else if (strcmp(config->logging.level, "error") == 0) {
        current_log_level = ERROR;
    }

    if (strcmp(config->logging.format, "json") == 0) {
        is_json_format = 1;
    }
}

void log_message(LogLevel level, const char *message) {
    if (level < current_log_level) {
        return;
    }

    time_t now;
    time(&now);
    struct tm *local = localtime(&now);

    const char *level_str;
    switch (level) {
        case DEBUG: level_str = "DEBUG"; break;
        case INFO: level_str = "INFO"; break;
        case WARN: level_str = "WARN"; break;
        case ERROR: level_str = "ERROR"; break;
        default: level_str = "UNKNOWN"; break;
    }

    if (is_json_format) {
        printf("{\"time\":\"%02d:%02d:%02d\",\"level\":\"%s\",\"message\":\"%s\"}\n",
               local->tm_hour, local->tm_min, local->tm_sec, level_str, message);
    } else {
        printf("%02d:%02d:%02d [%s] %s\n",
               local->tm_hour, local->tm_min, local->tm_sec, level_str, message);
    }
}
