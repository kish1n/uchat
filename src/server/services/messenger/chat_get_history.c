#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../pkg/config/config.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/users/users.h"

int handle_get_chat_history(HttpContext *context) {
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_get_chat_history");
        return MHD_NO;
    }

    const char *url = context->url;
    int chat_id = atoi(url);

    if (chat_id <= 0) {
        return prepare_simple_response("Invalid or missing 'chat_id'", STATUS_BAD_REQUEST, NULL, context);
    }


    char *username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &username) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_simple_response("Invalid JWT", STATUS_UNAUTHORIZED, NULL, context);
    }

    User *sender = get_user_by_username(context->db_conn, username);
    free(username);

    if (!sender) {
        logging(ERROR, "User not found: %s", username);
        return prepare_simple_response("User not found", STATUS_NOT_FOUND, NULL, context);
    }

    // Check if chat exists
    if (!chat_exists(context->db_conn, chat_id)) {
        logging(ERROR, "Chat does not exist: %d", chat_id);
        int ret = prepare_simple_response("Chat not found", STATUS_NOT_FOUND, NULL, context);
        free_user(sender);
        return ret;
    }

    // Check if user is part of the chat
    if (!is_user_in_chat(context->db_conn, chat_id, sender->id)) {
        logging(ERROR, "User '%s' is not in chat ID '%d'", sender->username, chat_id);
        int ret = prepare_simple_response("User is not part of the chat", STATUS_FORBIDDEN, NULL, context);
        free_user(sender);
        return ret;
    }

    char *messages_json = get_chat_messages(context->db_conn, chat_id, &cfg);
    if (!messages_json) {
        logging(ERROR, "Failed to retrieve chat messages for chat ID: %d", chat_id);
        prepare_simple_response("Failed to retrieve chat messages", STATUS_INTERNAL_SERVER_ERROR, NULL, context);
    }

    return prepare_response(STATUS_OK, json_tokener_parse(messages_json), context);
}