#ifndef HTTPCONTEXT_H
#define HTTPCONTEXT_H

#include <microhttpd.h>
#include <sqlite3.h>

typedef struct {
    void *cls;
    struct MHD_Connection *connection;
    const char *url;
    const char *method;
    const char *version;
    const char *upload_data;
    size_t *upload_data_size;
    void **con_cls;
    sqlite3 *db_conn;
} HttpContext;

#endif // HTTPCONTEXT_H