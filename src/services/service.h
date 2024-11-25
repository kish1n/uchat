#ifndef SERVICE_H
#define SERVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <libpq-fe.h>

#define MAX_HANDLERS 100
#define MAX_PATH_LEN 256
#define PORT 8080

typedef struct {
    void *cls;
    struct MHD_Connection *connection;
    const char *url;
    const char *method;
    const char *version;
    const char *upload_data;
    size_t *upload_data_size;
    void **con_cls;
    PGconn *db_conn;
} HttpContext;

typedef struct {
    int port;
    struct MHD_Daemon *daemon;
    PGconn *db_conn;
} Server;

typedef struct {
    char *data;
    size_t size;
} RequestData;

Server *server_init(int port, PGconn *db_conn);
int server_start(Server *server);
void server_stop(Server *server);
void server_destroy(Server *server);

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
