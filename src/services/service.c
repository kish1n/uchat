#include "service.h"
#include <stdio.h>
#include <stdlib.h>
#include "../pkg/config/config.h"
#include <microhttpd.h>

Service* service_create(const char *config_path) {
    Service *service = (Service *)malloc(sizeof(Service));
    if (!service) {
        fprintf(stderr, "Error: could not allocate memory for Service\n");
        return NULL;
    }

    if (load_config(config_path, &service->config) != 0) {
        fprintf(stderr, "Error: failed to load configuration\n");
        free(service);
        return NULL;
    }

    service->db_connection = PQconnectdb(service->config.database.url);
    if (PQstatus(service->db_connection) != CONNECTION_OK) {
        fprintf(stderr, "Error: failed to connect to the database: %s\n", PQerrorMessage(service->db_connection));
        free(service);
        return NULL;
    }

    service->http_port = service->config.server.port ? atoi(service->config.server.port) : 8888;
    service->http_daemon = NULL;
    service->is_running = 1;

    printf("Service created and initialized successfully\n");
    return service;
}

void service_start(Service *service) {
    if (!service || !service->is_running) {
        fprintf(stderr, "Error: service is not initialized or already stopped\n");
        return;
    }

    service->http_daemon = MHD_start_daemon(
        MHD_USE_INTERNAL_POLLING_THREAD,
        service->http_port,
        NULL, NULL,
        &request_handler,
        (void *)service,
        MHD_OPTION_END
    );

    if (service->http_daemon == NULL) {
        fprintf(stderr, "Error: failed to start HTTP server\n");
        service->is_running = 0;
        return;
    }

    printf("HTTP server started on port %d\n", service->http_port);
}

void service_stop(Service *service) {
    if (!service || !service->is_running) {
        return;
    }

    if (service->http_daemon) {
        MHD_stop_daemon(service->http_daemon);
        printf("HTTP server stopped\n");
    }

    if (service->db_connection) {
        PQfinish(service->db_connection);
        printf("Database connection closed\n");
    }

    free(service);
    printf("Service stopped and resources freed\n");
}
