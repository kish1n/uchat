#include "service.h"
#include <stdlib.h>
#include <stdio.h>
#include "../db/core/core.h"
#include "../pkg/config/config.h"
#include "../pkg/logger/logger.h"

Service* service_create(const char *config_path) {
    Service *service = (Service *)malloc(sizeof(Service));
    if (!service) {
        fprintf(stderr, "Error: could not allocate memory for service\n");
        return NULL;
    }

    if (load_config(config_path, &service->config) != 0) {
        fprintf(stderr, "Error: failed to load configuration\n");
        free(service);
        return NULL;
    }

    init_logger(&service->config);

    service->db_connection = connect_db(service->config.database.url);
    if (!service->db_connection) {
        fprintf(stderr, "Error: failed to connect to the database\n");
        free(service);
        return NULL;
    }

    log_message(INFO, "Service created and initialized successfully");
    return service;
}

void service_start(Service *service) {
    if (!service) {
        fprintf(stderr, "Error: service is not initialized\n");
        return;
    }

    log_message(INFO, "Service started successfully");
}

void service_stop(Service *service) {
    if (!service) {
        return;
    }

    disconnect_db(service->db_connection);
    log_message(INFO, "Database connection closed");

    free(service);
    log_message(INFO, "Service stopped and resources freed");
}
