#include "service.h"
#include <libwebsockets.h>
#include <stdlib.h>
#include <json-c/json.h>
#include "../services/service.h"
#include "../pkg/httputils/httputils.h"
#include "../pkg/http_response/response.h"
#include "../pkg/ws/ws.h"
#include "../services/messenger/messenger.h"
#include "../db/core/core.h"
#include "auth/auth_handlers.h"

enum MHD_Result router(void *cls,
                       struct MHD_Connection *connection,
                       const char *url,
                       const char *method,
                       const char *version,
                       const char *upload_data,
                       size_t *upload_data_size,
                       void **con_cls) {
    Config config;
    if (load_config("config.yaml", &config) != 0) {
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
        if (strcmp(sub_url, "/logout") == 0 && strcmp(method, "GET") == 0) {
            return handle_logout(&context);
        }
        if (strcmp(sub_url, "/update_username") == 0 && strcmp(method, "PUT") == 0) {
            return handle_update_username(&context);
        }

    } else if (starts_with(url, "/messages/")) {
        const char *sub_url = url + strlen("/messages");

        if (strcmp(sub_url, "/send") == 0 && strcmp(method, "POST") == 0) {
            return handle_send_message(&context);
        }

        const char *id_str = sub_url + strlen("/history/");
        int chat_id = atoi(id_str);
        if (chat_id > 0) {
            context.url = id_str;
            return handle_get_chat_history(&context);
        } else {
            const char *error_msg = create_error_response("Invalid or missing 'chat_id' (router)", STATUS_BAD_REQUEST);
            struct MHD_Response *response = MHD_create_response_from_buffer(
                strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
            int ret = MHD_queue_response(context.connection, MHD_HTTP_BAD_REQUEST, response);
            MHD_destroy_response(response);
            return ret;
        }

    } else if (starts_with(url, "/chats/")) {
        const char *sub_url = url + strlen("/chats");

        if (strcmp(sub_url, "/create") == 0 && strcmp(method, "POST") == 0) {
            return handle_create_private_chat(&context);
        }
        if (strcmp(sub_url, "/create_group") == 0 && strcmp(method, "POST") == 0) {
            return handle_create_group_chat(&context);
        }
        if (strcmp(sub_url, "/update_name") == 0 && strcmp(method, "PUT") == 0) {
            return handle_update_chat_name(&context);
        }
        if (strcmp(sub_url, "/add_member") == 0 && strcmp(method, "POST") == 0) {
            return handle_add_member_to_chat(&context);
        }
        if (strcmp(sub_url, "/remove_member") == 0 && strcmp(method, "DELETE") == 0) {
            return handle_remove_member_from_chat(&context);
        }
        if (strcmp(sub_url, "/leave") == 0 && strcmp(method, "DELETE") == 0) {
            return handle_leave_chat(&context);
        }
        if (strcmp(sub_url, "/delete") == 0 && strcmp(method, "DELETE") == 0) {
            return handle_delete_chat(&context);
        }

        // Handle dynamic URL for chat info: /chat/info/{chat_id}
        if (starts_with(sub_url, "/info/") && strcmp(method, "GET") == 0) {
            const char *id_str = sub_url + strlen("/info/");
            int chat_id = atoi(id_str);
            if (chat_id > 0) {
                context.url = id_str;
                return handle_get_chat_info(&context);
            } else {
                const char *error_msg = create_error_response("Invalid or missing 'chat_id' (router)", STATUS_BAD_REQUEST);
                struct MHD_Response *response = MHD_create_response_from_buffer(
                    strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
                int ret = MHD_queue_response(context.connection, MHD_HTTP_BAD_REQUEST, response);
                MHD_destroy_response(response);
                return ret;
            }
        }

    } else if (starts_with(url, "/user/")) {
        const char *sub_url = url + strlen("/user");

        if (strcmp(sub_url, "/chats") == 0 && strcmp(method, "GET") == 0) {
            return handle_get_user_chats(&context);
        }
    }

    const char *error_msg = create_error_response("Endpoint not found", STATUS_NOT_FOUND);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}
