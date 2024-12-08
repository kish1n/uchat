#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../pkg/config/config.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/users/users.h"

int handle_remove_member_from_chat(HttpContext *context) {
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_remove_member_from_chat");
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


    struct json_object *chat_id_obj, *username_obj;
    const char *username = NULL;
    int chat_id = -1;

    // Extract required fields
    if (json_object_object_get_ex(parsed_json, "chat_id", &chat_id_obj)) {
        chat_id = json_object_get_int(chat_id_obj);
    }

    if (json_object_object_get_ex(parsed_json, "username", &username_obj)) {
        username = json_object_get_string(username_obj);
    }

    // Validate required fields
    if (chat_id <= 0 || !username || strlen(username) == 0) {
        return prepare_response("Missing or invalid 'chat_id' or 'username'", STATUS_BAD_REQUEST, parsed_json, context);
    }

    char *sender = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &sender) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_response("Invalid JWT", STATUS_UNAUTHORIZED, parsed_json, context);
    }

    User *admin_user = get_user_by_username(context->db_conn, sender);

    // Check if the user is an admin in the chat
    if (!is_user_in_chat(context->db_conn, chat_id, admin_user->id) ||
        !is_user_admin(context->db_conn, chat_id, admin_user->id)) {
        logging(ERROR, "User '%s' is not an admin in chat ID '%d'", admin_user->username, chat_id);
        return prepare_response("User is not an admin in the chat", STATUS_FORBIDDEN, parsed_json, context);
    }

    // Find the user to be removed by username
    User *target_user = get_user_by_username(context->db_conn, username);
    if (!target_user || !target_user->id) {
        logging(ERROR, "User '%s' not found", username);
        return prepare_response("User not found", STATUS_NOT_FOUND, parsed_json, context);
    }

    // Prevent admin from kicking themselves
    if (strcmp(admin_user->id, target_user->id) == 0) {
        logging(ERROR, "Admin '%s' tried to remove themselves from chat ID '%d'", admin_user->username, chat_id);
        int ret = prepare_response("Admin cannot remove themselves", STATUS_FORBIDDEN, parsed_json, context);

        free_user(target_user);
        free_user(admin_user);
        free(sender);

        return ret;
    }

    // Remove the user from the chat
    if (delete_user_from_chat(context->db_conn, chat_id, target_user->id) != 0) {
        logging(ERROR, "Failed to remove user '%s' from chat ID '%d'", username, chat_id);
        int ret = prepare_response("Admin cannot remove themselves", STATUS_FORBIDDEN, parsed_json, context);

        free(sender);
        free_user(target_user);
        free_user(admin_user);

        return ret;
    }

    // Successful response
    int ret = prepare_response("User removed from chat", STATUS_OK, parsed_json, context);

    logging(INFO, "User '%s' removed from chat ID '%d' by admin '%s'", username, chat_id, admin_user->username);

    free(sender);
    free_user(admin_user);
    free_user(target_user);

    return ret;
}
