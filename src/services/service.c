#include "service.h"
#include <stdio.h>
#include <stdlib.h>
#include <microhttpd.h>
#include <string.h>

// Создание сервиса
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

// Инициализация сервиса
void service_init(Service *service) {
    printf("Service initialized with JWT secret: %s\n", service->jwt_secret);
}

// Регистрация эндпоинта
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

// Запуск сервиса
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
        sleep(1); // Поддерживаем сервис в рабочем состоянии
    }

    MHD_stop_daemon(service->daemon);
}

// Остановка сервиса
void service_stop(Service *service) {
    if (!service->running) {
        printf("Service is not running.\n");
        return;
    }

    service->running = 0;
}

// Уничтожение сервиса
void service_destroy(Service *service) {
    if (!service) return;
    if (service->running) {
        service_stop(service);
    }
    free(service);
}

// Обработка запросов
static int handle_request(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                          const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
    Service *service = (Service *)cls;

    // Поиск подходящего обработчика
    for (int i = 0; i < service->handler_count; i++) {
        EndpointHandler *handler = &service->handlers[i];
        if (strcmp(handler->path, url) == 0 && strcmp(handler->method, method) == 0) {
            return handler->handler(connection);
        }
    }

    // Если обработчик не найден, возвращаем 404
    const char *not_found = "{\"error\": \"Endpoint not found\"}";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(not_found), (void *)not_found, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

// Пример обработчиков
int login_handler(struct MHD_Connection *connection) {
    const char *response = "{\"message\": \"Login successful\"}";
    struct MHD_Response *http_response = MHD_create_response_from_buffer(strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, http_response);
    MHD_destroy_response(http_response);
    return ret;
}

int register_handler(struct MHD_Connection *connection) {
    const char *response = "{\"message\": \"Register successful\"}";
    struct MHD_Response *http_response = MHD_create_response_from_buffer(strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, http_response);
    MHD_destroy_response(http_response);
    return ret;
}