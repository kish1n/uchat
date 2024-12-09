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

enum MHD_Result handle_request(HttpContext *context) {
    
    /*HttpContext context = {
        .cls = cls,
        .connection = connection,
        .url = url,
        .method = method,
        .version = version,
        .upload_data = upload_data,
        .upload_data_size = upload_data_size,
        .con_cls = con_cls,
        .db_conn = db
    };*/

    logging(DEBUG, "Router invoked for URL: %s", context->url);
    if (!context->url) {
        logging(DEBUG, "Received NULL URL in router");
        return MHD_NO;
    }
    Config config;
    if (load_config("config.yaml", &config) != 0) {
        logging(ERROR, "Failed to load config");

        return EXIT_FAILURE;
    }

    if (!context->db_conn) {
        if (init_db("uchat.db") != SQLITE_OK) {
            logging(ERROR, "Failed to initialize database");
            return EXIT_FAILURE;
        }
        context->db_conn = get_db();
    }

    

    if (starts_with(context->url, "/auth/")) {
        const char *sub_url = context->url + strlen("/auth");

        if (strcmp(sub_url, "/register") == 0 && strcmp(context->method, "POST") == 0) {
            return handle_register(context); //{"username": "username","password": "password"}
        }
        if (strcmp(sub_url, "/login") == 0 && strcmp(context->method, "POST") == 0) {
            return handle_login(context); //{"username": "username","password": "password"}
        }
        if (strcmp(sub_url, "/logout") == 0 && strcmp(context->method, "GET") == 0) {
            return handle_logout(context); //GET
        }
        if (strcmp(sub_url, "/update_username") == 0 && strcmp(context->method, "PUT") == 0) {
            return handle_update_username(context); //{"username": "new_username"}
        }

    } else if (starts_with(context->url, "/messages/")) {
        const char *sub_url = context->url + strlen("/messages");

        if (strcmp(sub_url, "/send") == 0 && strcmp(context->method, "POST") == 0) {
            return handle_send_message(context); //{"chat_id": "chat_id", "content": "message"}
        }

        const char *id_str = sub_url + strlen("/history/");
        int chat_id = atoi(id_str);
        if (chat_id > 0) {
            context->url = id_str;
            return handle_get_chat_history(context); //GET
        }

        return prepare_simple_response("Invalid or missing 'chat_id'", STATUS_BAD_REQUEST, NULL, context);

    } else if (starts_with(context->url, "/chats/")) {
        const char *sub_url = context->url + strlen("/chats");

        if (strcmp(sub_url, "/create_private") == 0 && strcmp(context->method, "POST") == 0) {
            return handle_create_private_chat(context); //{"with_user": "username"}
        }
        if (strcmp(sub_url, "/create_group") == 0 && strcmp(context->method, "POST") == 0) {
            return handle_create_group_chat(context); //{"name": "chat_name", "users": ["username1", "username2", ...]}
        }
        if (strcmp(sub_url, "/update_name") == 0 && strcmp(context->method, "PUT") == 0) {
            return handle_update_chat_name(context); //{"chat_id": "chat_id", "name": "new_name"}
        }
        if (strcmp(sub_url, "/add_member") == 0 && strcmp(context->method, "POST") == 0) {
            return handle_add_member_to_chat(context); //{"chat_id": "chat_id", "username": "username"}
        }
        if (strcmp(sub_url, "/remove_member") == 0 && strcmp(context->method, "DELETE") == 0) {
            return handle_remove_member_from_chat(context); //{"chat_id": "chat_id", "username": "username"}
        }
        if (strcmp(sub_url, "/leave") == 0 && strcmp(context->method, "DELETE") == 0) {
            return handle_leave_chat(context); //{"chat_id": "chat_id"}
        }
        if (strcmp(sub_url, "/delete") == 0 && strcmp(context->method, "DELETE") == 0) {
            return handle_delete_chat(context); //{"chat_id": "chat_id"}
        }

        // Handle dynamic URL for chat info: /chat/info/{chat_id}
        if (starts_with(sub_url, "/info/") && strcmp(context->method, "GET") == 0) {
            const char *id_str = sub_url + strlen("/info/");
            int chat_id = atoi(id_str);
            if (chat_id > 0) {
                context->url = id_str;
                return handle_get_chat_info(context);
            }

            return prepare_simple_response("Invalid or missing 'chat_id'", STATUS_BAD_REQUEST, NULL, context);

        }

        if (starts_with(sub_url, "/poll/") && strcmp(context->method, "GET") == 0) {
            const char *id_str = sub_url + strlen("/poll/");
            int chat_id = atoi(id_str);

            if (chat_id > 0) {
                return handle_long_polling(context, context->connection, chat_id);
            } else {
                const char *error_msg = create_error_response("Invalid or missing 'chat_id' (router)", STATUS_BAD_REQUEST);
                struct MHD_Response *response = MHD_create_response_from_buffer(
                    strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
                int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
                MHD_destroy_response(response);
                return ret;
            }
        }

    } else if (starts_with(context->url, "/user/")) {
        const char *sub_url = context->url + strlen("/user");

        if (strcmp(sub_url, "/chats") == 0 && strcmp(context->method, "GET") == 0) {
            return handle_get_user_chats(context);
        }
    }
    
    if (starts_with(context->url, "/test-url")) {
        logging(INFO, "Received test-url");
    }

    return prepare_simple_response("Endpoint not found", STATUS_NOT_FOUND, NULL, context);
}

