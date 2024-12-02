#include "service.h"
#include <stdlib.h>
#include <json-c/json.h>
#include "../db/core/core.h"
#include "../pkg/httputils/httputils.h"

#include "auth/auth_handlers.h"
#include "messages/messages.h"
#include "chats/chats.h"

enum MHD_Result router(void *cls,
                               struct MHD_Connection *connection,
                               const char *url,
                               const char *method,
                               const char *version,
                               const char *upload_data,
                               size_t *upload_data_size,
                               void **con_cls) {

    Config config;
    if (load_config("../config.yaml", &config) != 0) {
        logging(ERROR, "Failed to load config");

        return EXIT_FAILURE;
    }

    PGconn *db_conn = connect_db(config.database.url);
    if (db_conn == NULL || PQstatus(db_conn) != CONNECTION_OK) {
        logging(ERROR, "Failed to connect to database: %s", PQerrorMessage(db_conn));
        if (db_conn) disconnect_db(db_conn);
        return EXIT_FAILURE;
    }

    HttpContext context = {
        .cls = cls,
        .connection = connection,
        .url = url,
        .method = method,
        .version = version,
        .upload_data = upload_data,
        .upload_data_size = upload_data_size,
        .con_cls = con_cls,
        .db_conn = db_conn
    };

    if (starts_with(url, "/auth/")) {
        const char *sub_url = url + strlen("/auth");

        if (strcmp(sub_url, "/register") == 0 && strcmp(method, "POST") == 0) {
            return handle_register(&context);
        }
        if (strcmp(sub_url, "/login") == 0 && strcmp(method, "POST") == 0) {
            return handle_login(&context);
        }
    } else if (starts_with(url, "/messages/")) {
        const char *sub_url = url + strlen("/messages");

        if (strcmp(sub_url, "/send") == 0 && strcmp(method, "POST") == 0) {
            return handle_send_message(&context);
        }
    } else if (starts_with(url, "/chats/")) {
        const char *sub_url = url + strlen("/chats");

        if (strcmp(sub_url, "/create") == 0 && strcmp(method, "POST") == 0) {
            return handle_create_chat(&context);
        }
    }

    const char *error_msg = "Endpoint not found";
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}