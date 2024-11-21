#include "service.h"

Service *service_create(const char *jwt_secret) {
    Service *service = (Service *)malloc(sizeof(Service));
    if (!service) {
        fprintf(stderr, "Failed to allocate memory for Service\n");
        return NULL;
    }

    service->running = 0;
    service->handler_count = 0;
    strncpy(service->jwt_secret, jwt_secret, sizeof(service->jwt_secret) - 1);
    service->jwt_secret[sizeof(service->jwt_secret) - 1] = '\0';
    service->daemon = NULL;

    return service;
}

void service_init(Service *service) {
    printf("Service initialized with JWT secret: %s\n", service->jwt_secret);
}

void service_register_endpoint(Service *service, const char *path, const char *method, int (*handler)(struct MHD_Connection *)) {
    if (service->handler_count >= MAX_HANDLERS) {
        fprintf(stderr, "Cannot register more endpoints, limit reached.\n");
        return;
    }

    EndpointHandler *new_handler = &service->handlers[service->handler_count++];
    strncpy(new_handler->path, path, MAX_PATH_LEN - 1);
    new_handler->path[MAX_PATH_LEN - 1] = '\0';
    new_handler->method = method;
    new_handler->handler = handler;

    printf("Registered endpoint: %s %s\n", method, path);
}

void service_start(Service *service) {
    if (service->running) {
        printf("Service is already running.\n");
        return;
    }

    printf("Starting service on port %d...\n", PORT);
    service->running = 1;

    service->daemon = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &handle_request, service, MHD_OPTION_END);

    if (!service->daemon) {
        fprintf(stderr, "Failed to start HTTP server\n");
        service->running = 0;
        return;
    }

    printf("Service started. Press Ctrl+C to stop.\n");
    while (service->running) {
        sleep(1);
    }

    MHD_stop_daemon(service->daemon);
}

void service_stop(Service *service) {
    if (!service->running) {
        printf("Service is not running.\n");
        return;
    }

    service->running = 0;
}

void service_destroy(Service *service) {
    if (!service) return;
    if (service->running) {
        service_stop(service);
    }
    free(service);
}

