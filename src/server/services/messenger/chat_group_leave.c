#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../pkg/config/config.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/users/users.h"

int handle_leave_chat(HttpContext *context) {
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

    // Extract required fields
    if (json_object_object_get_ex(parsed_json, "chat_id", &chat_id_obj)) {
        chat_id = json_object_get_int(chat_id_obj);
    }

    // Validate required fields
    if (chat_id <= 0) {
        return prepare_response("Missing or invalid 'chat_id'", STATUS_BAD_REQUEST, parsed_json, context);
    }

    char *sender = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &sender) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_response("Invalid JWT", STATUS_UNAUTHORIZED, parsed_json, context);
    }

    User *user = get_user_by_username(context->db_conn, sender);

    // Check if the user is in the chat
    if (!is_user_in_chat(context->db_conn, chat_id, user->id)) {
        int ret = prepare_response("User is not in the chat", STATUS_FORBIDDEN, parsed_json, context);

        free_user(user);
        free(sender);

        return ret;
    }

    // Check if the user is the admin
    if (is_user_admin(context->db_conn, chat_id, user->id)) {
        logging(ERROR, "Admin '%s' cannot leave the chat", user->username);
        int ret = prepare_response("Admin cannot leave the chat", STATUS_FORBIDDEN, parsed_json, context);
        free_user(user);
        free(sender);
        return ret;
    }

    // Remove the user from the chat
    if (delete_user_from_chat(context->db_conn, chat_id, user->id) != 0) {
        logging(ERROR, "Failed to remove user '%s' from chat ID '%d'", user->username, chat_id);
        int ret = prepare_response("Failed to leave the chat", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);
        free_user(user);
        free(sender);
        return ret;
    }

    // Successful response
    int ret = prepare_response("User left the chat", STATUS_OK, parsed_json, context);

    logging(INFO, "User '%s' left chat ID '%d'", user->username, chat_id);

    free_user(user);
    free(sender);

    return ret;
}
