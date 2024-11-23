#ifndef AUTH_HANDLERS_H
#define AUTH_HANDLERS_H

#include <microhttpd.h>
#include <libpq-fe.h>

enum MHD_Result handle_register(void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **con_cls,
    PGconn *db_conn);

enum MHD_Result handle_login(void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **con_cls,
    PGconn *db_conn);

#endif // AUTH_HANDLERS_H
