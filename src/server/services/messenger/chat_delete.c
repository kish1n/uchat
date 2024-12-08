#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../db/core/chats/chats.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../pkg/http_response/response.h"
#include "../../pkg/httputils/httputils.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../db/core/users/users.h"
#include "../../db/core/messages/messages.h"

int handle_delete_chat(HttpContext *context) {
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_leave_chat");
        return MHD_NO;
    }

    // Allocate memory for incoming data if not already allocated
    if (*context->con_cls == NULL) {
        char *buffer = calloc(1, sizeof(char));
        *context->con_cls = buffer;
        return MHD_YES;
    }

    char *data = (char *)*context->con_cls;

    // Accumulate incoming data
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

    struct json_object *chat_id_obj;
    int chat_id = -1;

    if (json_object_object_get_ex(parsed_json, "chat_id", &chat_id_obj)) {
        chat_id = json_object_get_int(chat_id_obj);
    }

    // Validate required fields
    if (chat_id <= 0) {
        return prepare_response("Missing or invalid 'id' field", STATUS_BAD_REQUEST, parsed_json, context);
    }

    // Check if the chat exists
    if (!chat_exists(context->db_conn, chat_id)) {
        return prepare_response("Chat does not exist", STATUS_NOT_FOUND, parsed_json, context);
    }

    // Extract and verify JWT
    char *username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &username) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_response("JWT verification failed", STATUS_UNAUTHORIZED, parsed_json, context);
    }

    User *user = get_user_by_username(context->db_conn, username);

    // Check if the user is an admin in the chat
    if (!is_user_in_chat(context->db_conn, chat_id, user->id) ||
        !is_user_admin(context->db_conn, chat_id, user->id)) {
        logging(ERROR, "User '%s' is not an admin in chat ID '%d'", user->username, chat_id);
        return prepare_response("User is not an admin in the chat", STATUS_UNAUTHORIZED, parsed_json, context);
        }


    // Delete the chat from the database
    int result1 = delete_chat(context->db_conn, chat_id);
    int result2 = delete_all_chat_members(context->db_conn, chat_id);
    int result3 = delete_all_messages_in_chat(context->db_conn, chat_id);

    json_object_put(parsed_json);
    free(username);

    if (result1 == 0 && result2 == 0 && result3 == 0) {
        logging(INFO, "Chat with id %d deleted successfully", chat_id);
        return prepare_response("Chat deleted successfully", STATUS_OK, NULL, context);
    }
        logging(ERROR, "Failed to delete chat with id %d", chat_id);
        return prepare_response("Failed to delete chat", STATUS_INTERNAL_SERVER_ERROR, NULL, context);
}
