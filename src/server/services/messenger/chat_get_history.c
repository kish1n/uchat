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

    // Check if chat exists
    if (!chat_exists(context->db_conn, chat_id)) {
        logging(ERROR, "Chat does not exist: %d", chat_id);
        const char *error_msg = create_error_response("Chat does not exist", STATUS_NOT_FOUND);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        free_user(sender);
        return ret;
    }

    // Check if user is part of the chat
    if (!is_user_in_chat(context->db_conn, chat_id, sender->id)) {
        logging(ERROR, "User '%s' is not in chat ID '%d'", sender->username, chat_id);
        const char *error_msg = create_error_response("forbidden", STATUS_FORBIDDEN);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_FORBIDDEN, response);
        MHD_destroy_response(response);
        free_user(sender);
        return ret;
    }

    char *messages_json = get_chat_messages(context->db_conn, chat_id);
    if (!messages_json) {
        logging(ERROR, "Failed to retrieve chat messages for chat ID: %d", chat_id);
        const char *error_msg = create_error_response("Chat not found or no messages available", STATUS_NOT_FOUND);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        return MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
    }

    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(messages_json), (void *)messages_json, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
    free(messages_json);
    MHD_destroy_response(response);
    logging(INFO, "Chat history retrieved successfully for chat ID: %d", chat_id);
    return ret;

}