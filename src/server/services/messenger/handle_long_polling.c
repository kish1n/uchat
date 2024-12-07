#include "messenger.h"
#include "../../db/core/messages/messages.h"
#include "../../db/core/users/users.h"
#include "../../db/core/core.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../pkg/http_response/response.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Для sleep
#include <json-c/json.h>

#define POLL_INTERVAL 1
#define MAX_POLL_TIME 30

int handle_long_polling(HttpContext *context, struct MHD_Connection *connection, int chat_id) {
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_get_chat_history");
        return MHD_NO;
    }

    if (chat_id <= 0) {
        const char *error_msg = "Invalid or missing 'chat_id'";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }


    char *username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &username) != 1) {
        logging(ERROR, "JWT verification failed");
        const char *error_msg = create_error_response("unauthorized", STATUS_UNAUTHORIZED);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_UNAUTHORIZED, response);
        MHD_destroy_response(response);
        return ret;
    }

    User *sender = get_user_by_username(context->db_conn, username);
    free(username);

    if (!sender) {
        logging(ERROR, "User not found: %s", username);
        const char *error_msg = create_error_response("User not found", STATUS_NOT_FOUND);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret;
    }

    struct json_object *response_array = json_object_new_array();
    int elapsed_time = 0;

    while (elapsed_time < MAX_POLL_TIME) {
        Message *messages = NULL;
        int message_count = 0;

        if (get_messages_by_chat(get_db(), chat_id, &messages, &message_count) == 0) {
            if (message_count > 0) {
                for (int i = 0; i < message_count; i++) {
                    struct json_object *message_obj = json_object_new_object();
                    json_object_object_add(message_obj, "id", json_object_new_int(messages[i].id));
                    json_object_object_add(message_obj, "chat_id", json_object_new_int(messages[i].chat_id));
                    json_object_object_add(message_obj, "sender_id", json_object_new_string(messages[i].sender_id));
                    json_object_object_add(message_obj, "content", json_object_new_string(messages[i].content));
                    json_object_object_add(message_obj, "sent_at", json_object_new_string(messages[i].sent_at));

                    json_object_array_add(response_array, message_obj);
                }

                free(messages);
                break;
            }
        }

        free(messages);
        sleep(POLL_INTERVAL);
        elapsed_time += POLL_INTERVAL;
    }

    const char *response_str = json_object_to_json_string(response_array);
    struct MHD_Response *mhd_response = MHD_create_response_from_buffer(
        strlen(response_str), (void *)response_str, MHD_RESPMEM_MUST_COPY);
    json_object_put(response_array); // Освобождаем JSON-объект

    int status_code = (elapsed_time < MAX_POLL_TIME) ? MHD_HTTP_OK : MHD_HTTP_NO_CONTENT;
    int ret = MHD_queue_response(connection, status_code, mhd_response);
    MHD_destroy_response(mhd_response);

    return ret;
}
