#include "service.h"
#include <stdlib.h>
#include <json-c/json.h>
#include "../services/service.h"       // Ваш заголовочный файл для структуры Server
#include "../db/core/core.h"
#
#include "auth/auth_handlers.h"

enum MHD_Result handle_request(void *cls,
                               struct MHD_Connection *connection,
                               const char *url,
                               const char *method,
                               const char *version,
                               const char *upload_data,
                               size_t *upload_data_size,
                               void **con_cls) {

    Config config;
    if (load_config("../config.yaml", &config) != 0) {
        fprintf(stderr, "Failed to load config\n");
        return EXIT_FAILURE;
    }

    PGconn *db_conn = connect_db(config.database.url);
    if (db_conn == NULL || PQstatus(db_conn) != CONNECTION_OK) {
        fprintf(stderr, "Failed to connect to database: %s\n", PQerrorMessage(db_conn));
        if (db_conn) disconnect_db(db_conn);
        return EXIT_FAILURE;
    }

    if (strcmp(url, "/register") == 0 && strcmp(method, "POST") == 0) {
        return handle_register(cls, connection, url, method, version, upload_data, upload_data_size, con_cls, db_conn);
    }

    const char *error_msg = "Endpoint not found";
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}
