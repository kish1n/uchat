#ifndef SERVICE_H
#define SERVICE_H

#include <libpq-fe.h>
#include "../pkg/config/config.h"
#include "../pkg/logger/logger.h"

typedef struct {
    PGconn *db_connection;
    Config config;
    int is_running;
} Service;

Service* service_create(const char *config_path);
void service_start(Service *service);
void service_stop(Service *service);

#endif