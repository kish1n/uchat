#ifndef LOGGER_H
#define LOGGER_H

#include "../config/config.h"

typedef enum {
    DEBUG,
    INFO,
    WARN,
    ERROR
} LogLevel;

void init_logger(const Config *config);
void log_message(LogLevel level, const char *message);

#endif // LOGGER_H
