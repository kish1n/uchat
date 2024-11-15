#include "service.h"
#include <stdio.h>
#include <stdlib.h>
#include "../pkg/config/config.h"
#include <microhttpd.h>
#include <libpq-fe.h>
#include <string.h>

#include "auth/handlers/auth_handlers.h"

enum MHD_Result request_handler(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls) {
    Auth *auth = (Auth *)cls;
    if (strcmp(url, "/api/v1/register") == 0 && strcmp(method, "POST") == 0) {
        return handle_register_request(auth, connection);
    }
    const char *error_message = "{\"error\": \"Not found\"}";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_message),
                                                                    (void *)error_message,
                                                                    MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Content-Type", "application/json");
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

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
