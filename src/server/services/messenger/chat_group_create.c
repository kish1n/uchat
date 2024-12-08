#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../pkg/config/config.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../db/core/users/users.h"

int handle_create_group_chat(HttpContext *context) {
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
        return prepare_simple_response("Invalid JSON", STATUS_BAD_REQUEST, NULL, context);
    }


    struct json_object *name_obj, *users_array;
    const char *name = NULL;

    if (json_object_object_get_ex(parsed_json, "name", &name_obj)) {
        name = json_object_get_string(name_obj);
    }
    if (!json_object_object_get_ex(parsed_json, "users", &users_array)) {
        return prepare_simple_response("Missing 'users' array", STATUS_BAD_REQUEST, parsed_json, context);
    }

    if (!name || strlen(name) == 0) {
        return prepare_simple_response("Missing or invalid 'name' field", STATUS_BAD_REQUEST, parsed_json, context);
    }

    int chat_id = create_chat(context->db_conn, name, 1);
    if (chat_id <= 0) {
        logging(ERROR, "Failed to create chat '%s'", name);
        return prepare_simple_response("Failed to create chat", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);
    }

    char *creator_username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &creator_username) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_simple_response("Invalid JWT", STATUS_UNAUTHORIZED, parsed_json, context);
    }

    User *creator = get_user_by_username(context->db_conn, creator_username);

    if (add_chat_member(context->db_conn, chat_id, creator->id, 1) != 0) {
        logging(ERROR, "Failed to add creator '%s' to chat '%s'", creator->username, name);
        return prepare_simple_response("Failed to add chat creator", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);
    }

    size_t user_count = json_object_array_length(users_array);
    for (size_t i = 0; i < user_count; ++i) {
        struct json_object *username_obj = json_object_array_get_idx(users_array, i);
        const char *username = json_object_get_string(username_obj);

        if (!username || strcmp(username, creator_username) == 0) continue;

        User *user = get_user_by_username(context->db_conn, username);
        if (!user) {
            logging(WARN, "User '%s' not found, skipping", username);
            continue;
        }

        if (add_chat_member(context->db_conn, chat_id, user->id, 0) != 0) {
            logging(WARN, "Failed to add user %s to chat %d", user->username, chat_id);
        }
        free_user(user);
    }

    free(creator_username);

    logging(INFO, "Group chat '%s' created successfully with ID %d", name, chat_id);
    return prepare_simple_response("Success", STATUS_OK, parsed_json, context);
}
