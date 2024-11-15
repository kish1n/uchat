#ifndef SERVICE_H
#define SERVICE_H

#include <libpq-fe.h>
#include "../pkg/config/config.h"
#include "../pkg/logger/logger.h"

typedef struct {
    PGconn *db_connection;          // db conn
    Config config;                  // cfg service
    struct MHD_Daemon *http_daemon; // ex HTTP-srv
    int http_port;                  // port for HTTP-srv
    int is_running;                 // status
} Service;

Service* service_create(const char *config_path);
void service_start(Service *service);
void service_stop(Service *service);

#endif