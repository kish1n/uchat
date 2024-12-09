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
        return prepare_simple_response("Invalid or missing 'chat_id'", STATUS_BAD_REQUEST, NULL, context);
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
        return prepare_simple_response("User not found", STATUS_NOT_FOUND, NULL, context);
    }

    int last_checked_message_id = get_last_message_id(context->db_conn, chat_id);
    if (last_checked_message_id < 0) {
        logging(ERROR, "Failed to fetch last_message_id for chat_id=%d", chat_id);
        return prepare_simple_response("Failed to fetch last_message_id", STATUS_INTERNAL_SERVER_ERROR, NULL, context);
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

    return prepare_response(STATUS_FOUND, response_array, context);
}
