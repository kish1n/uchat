#ifndef SERVICE_H
#define SERVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include "../pkg/config/config.h"
#include "../pkg/http_response/response.h"

#define MAX_HANDLERS 100
#define MAX_PATH_LEN 256
#define PORT 8080

typedef struct {
    char *data;
    size_t size;
} RequestData;

typedef struct WebSocketConnection {
    int client_id;
    struct MHD_Connection *connection;
    struct WebSocketConnection *next;
} WebSocketConnection;

typedef struct WebSocketServer {
    WebSocketConnection *clients;
    int client_count;
    pthread_mutex_t lock;
} WebSocketServer;

void free_request_data(void *con_cls);
enum MHD_Result router(void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **con_cls);

#endif
