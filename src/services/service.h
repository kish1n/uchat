#ifndef SERVICE_H
#define SERVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include "auth/handlers/handlers.h"

#define MAX_HANDLERS 100
#define MAX_PATH_LEN 256
#define PORT 8080

typedef struct {
    char path[MAX_PATH_LEN];                 // Путь эндпоинта (например, "/login")
    const char *method;                      // Метод HTTP (GET, POST и т.д.)
    int (*handler)(struct MHD_Connection *); // Функция-обработчик
} EndpointHandler;

typedef struct {
    int running;                             // Флаг состояния сервиса
    char jwt_secret[256];                    // Секрет для JWT
    EndpointHandler handlers[MAX_HANDLERS]; // Зарегистрированные эндпоинты
    int handler_count;                       // Количество зарегистрированных обработчиков
    struct MHD_Daemon *daemon;               // HTTP-демон
} Service;

// Прототипы функций
Service *service_create(const char *jwt_secret);
void service_init(Service *service);
void service_start(Service *service);
void service_stop(Service *service);
void service_destroy(Service *service);
void service_register_endpoint(Service *service, const char *path, const char *method, int (*handler)(struct MHD_Connection *));

enum MHD_Result handle_request(
    void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **con_cls
);

#endif
