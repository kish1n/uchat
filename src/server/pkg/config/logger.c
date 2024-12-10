#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "config.h"

#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RED     "\033[31m"

static LogLevel current_log_level = INFO;
static FILE *log_file = NULL;

static const char *log_level_to_string(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

static const char *log_level_to_color(LogLevel level) {
    switch (level) {
        case DEBUG: return COLOR_WHITE;
        case INFO: return COLOR_GREEN;
        case WARN: return COLOR_YELLOW;
        case ERROR: return COLOR_RED;
        default: return COLOR_RESET;
    }
}

void logging(LogLevel level, const char *format, ...) {
    if (level < current_log_level) {
        return;
    }

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char time_buffer[20];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local_time);

    char message_buffer[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(message_buffer, sizeof(message_buffer), format, args);
    va_end(args);

    fprintf(stdout, "%s[%s]%s [%s%s%s] %s\n",
            COLOR_BLUE, time_buffer, COLOR_RESET,
            log_level_to_color(level), log_level_to_string(level), COLOR_RESET,
            message_buffer);

    if (log_file) {
        fprintf(log_file, "[%s] [%s] %s\n", time_buffer, log_level_to_string(level), message_buffer);
        fflush(log_file);
    }
}

void init_logger(const LoggingConfig *logging_config) {
    if (strcmp(logging_config->level, "debug") == 0) {
        current_log_level = DEBUG;
    } else if (strcmp(logging_config->level, "info") == 0) {
        current_log_level = INFO;
    } else if (strcmp(logging_config->level, "warn") == 0) {
        current_log_level = WARN;
    } else if (strcmp(logging_config->level, "error") == 0) {
        current_log_level = ERROR;
    }

    if (strlen(logging_config->output_file) > 0) {
        log_file = fopen(logging_config->output_file, "a");
        if (!log_file) {
            fprintf(stderr, "Failed to open log file: %s. Using stderr\n", logging_config->output_file);
            log_file = stderr;
        }
    } else {
        log_file = stderr;
    }
}

void close_logger() {
    if (log_file && log_file != stderr) {
        fclose(log_file);
    }
    log_file = NULL;
}


