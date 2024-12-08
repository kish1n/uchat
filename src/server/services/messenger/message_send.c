#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../service.h"
#include "../../pkg/httputils/httputils.h"
#include "../../pkg/http_response/response.h"
#include "../../db/core/messages/messages.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../db/core/users/users.h"

int handle_send_message(HttpContext *context) {
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_create_group_chat");
        return MHD_NO;
    }

    if (*context->con_cls == NULL) {
        char *buffer = calloc(1, sizeof(char));
        *context->con_cls = buffer;
        return MHD_YES;
    }

    char *data = (char *)*context->con_cls;

    if (*context->upload_data_size > 0) {
        data = realloc(data, strlen(data) + *context->upload_data_size + 1);
        strncat(data, context->upload_data, *context->upload_data_size);
        *context->upload_data_size = 0;
        *context->con_cls = data;
        return MHD_YES;
    }

    struct json_object *parsed_json = json_tokener_parse(data);
    free(data);
    *context->con_cls = NULL;

    if (!parsed_json) {
        return prepare_response("Invalid JSON", STATUS_BAD_REQUEST, NULL, context);
    }


    struct json_object *chat_id_obj, *content_obj;
    int chat_id = -1;
    const char *content = NULL;

    if (json_object_object_get_ex(parsed_json, "chat_id", &chat_id_obj)) {
        chat_id = json_object_get_int(chat_id_obj);
    }

    if (json_object_object_get_ex(parsed_json, "content", &content_obj)) {
        content = json_object_get_string(content_obj);
    }

    // Validate fields
    if (chat_id <= 0 || !content || strlen(content) == 0) {
        const char *error_msg = "Missing or invalid 'chat_id' or 'content'";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Extract user from JWT
    char *username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &username) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_response("Invalid JWT", STATUS_UNAUTHORIZED, parsed_json, context);
    }

    User *sender = get_user_by_username(context->db_conn, username);
    free(username);

    if (!sender) {
        logging(ERROR, "User not found: %s", username);
        return prepare_response("User not found", STATUS_NOT_FOUND, parsed_json, context);
    }

    // Check if chat exists
    if (!chat_exists(context->db_conn, chat_id)) {
        logging(ERROR, "Chat does not exist: %d", chat_id);
        free_user(sender);

        return prepare_response("User not found", STATUS_NOT_FOUND, parsed_json, context);
    }

    // Check if user is part of the chat
    if (!is_user_in_chat(context->db_conn, chat_id, sender->id)) {
        logging(ERROR, "User '%s' is not in chat ID '%d'", sender->username, chat_id);
        free_user(sender);

        return prepare_response("User is not in the chat", STATUS_FORBIDDEN, parsed_json, context);
    }

    logging(DEBUG, "User '%s' is in chat ID '%d'", sender->username, chat_id);
    // Save message to database
    int result = create_message(context->db_conn, chat_id, sender->id, content);
    json_object_put(parsed_json);

    if (result < 0) {
        logging(ERROR, "Failed to create message");

        free_user(sender);
        return prepare_response("User is not in the chat", STATUS_FORBIDDEN, parsed_json, context);
    }

    logging(INFO, "Message sent by user '%s' in chat ID '%d'", sender->username, chat_id);
    free_user(sender);

    return prepare_response("Successfully sent message", STATUS_OK, NULL, context);
}
