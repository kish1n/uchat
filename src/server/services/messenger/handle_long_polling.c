#include "messenger.h"
#include "../../db/core/messages/messages.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/users/users.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../pkg/http_response/response.h"
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Для sleep

#define POLL_INTERVAL 1
#define MAX_POLL_TIME 10

int handle_long_polling(HttpContext *context, struct MHD_Connection *connection, int chat_id) {
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_long_polling");
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

    // Проверка JWT
    char *username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &username) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_simple_response("Invalid JWT", STATUS_UNAUTHORIZED, NULL, context);
    }

    User *user = get_user_by_username(context->db_conn, username);
    free(username);

    if (!user) {
        logging(ERROR, "User not found");
        const char *error_msg = create_error_response("User not found", STATUS_NOT_FOUND);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret;
    }

    int last_checked_message_id = get_last_message_id(context->db_conn, chat_id);
    if (last_checked_message_id < 0) {
        logging(ERROR, "Failed to fetch last_message_id for chat_id=%d", chat_id);
        const char *error_msg = create_error_response("Failed to fetch chat messages", STATUS_INTERNAL_SERVER_ERROR);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret;
    }

    struct json_object *response_array = json_object_new_array();
    int elapsed_time = 0;

    while (elapsed_time < MAX_POLL_TIME) {
        Message *messages = NULL;
        int message_count = 0;

        if (get_new_messages_by_chat(context->db_conn, chat_id, last_checked_message_id, &messages, &message_count) == 0) {
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

                // Обновляем `last_message_id` для чата
                edit_last_message_id(context->db_conn, chat_id, messages[message_count - 1].id);

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
    int ret = MHD_queue_response(context->connection, status_code, mhd_response);
    MHD_destroy_response(mhd_response);

    return ret;
}
