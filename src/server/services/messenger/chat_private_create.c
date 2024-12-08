#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../pkg/config/config.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../db/core/users/users.h"

int handle_create_private_chat(HttpContext *context) {
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_create_chat");
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
    const char *with_user = NULL;

    //LOGIC:

    if (json_object_object_get_ex(parsed_json, "with_user", &name_obj)) {
        with_user = json_object_get_string(name_obj);
    }

    char *cretaor_username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &cretaor_username) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_simple_response("Invalid JWT", STATUS_UNAUTHORIZED, parsed_json, context);
    }

    logging(INFO, "User ID: %s", cretaor_username);

    User *creator = get_user_by_username(context->db_conn, cretaor_username);
    User *second_user = get_user_by_username(context->db_conn, with_user);

    if (second_user == NULL || creator == NULL) {
        logging(ERROR, "Failed to get user by username");
        return prepare_simple_response("Failed to get user by username", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);
    }

    logging(DEBUG, "Creator: %s, Second user: %s", creator->username, second_user->username);

    if (!second_user->id) {
        logging(ERROR, "User with username '%s' not found", second_user->username);
        return prepare_simple_response("User not found", STATUS_NOT_FOUND, parsed_json, context);
    }

    logging(INFO, "User with username '%s' found", second_user->username);

    int chat_id = private_chat_exist(context->db_conn, creator->id, second_user->id);
    if (chat_id == -1) {
        logging(ERROR, "Failed to check if private chat exists");
        return prepare_simple_response("Failed to check if private chat exists", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);
    } else if (chat_id > 0) {
        logging(ERROR, "Private chat between '%s' and '%s' already exists", creator->username, second_user->username);
        return prepare_simple_response("already exists", STATUS_CONFLICT, parsed_json, context);
    }

    logging(INFO, "Creating private chat between '%s' and '%s'", creator->username, second_user->username);

    char *chat_name = malloc(strlen("private_") + strlen(creator->username) + strlen(second_user->username) + 2);
    if (!chat_name) {
        logging(ERROR, "Failed to allocate memory for chat name");
        int ret = prepare_simple_response("Failed to allocate memory for chat name", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);

        free_user(creator);
        free_user(second_user);

        return ret;
    }
    sprintf(chat_name, "private_%s_%s", creator->username, second_user->username);

    chat_id = create_chat(context->db_conn, chat_name, 0);
    if (chat_id <= 0) {
        logging(ERROR, "Failed to create chat '%s'", chat_name);
        int ret = prepare_simple_response("Failed to create chat", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);

        free_user(creator);
        free_user(second_user);
        free(chat_name);

        return ret;
    }

    if (add_chat_member(context->db_conn, chat_id, creator->id, 1) != 0) {
        logging(ERROR, "Failed to add creator '%s' to chat '%s'", creator->username, chat_name);
        int ret = prepare_simple_response("Failed to add chat creator", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);


        free_user(creator);
        free_user(second_user);
        free(chat_name);

        return ret;
    }

    if (add_chat_member(context->db_conn, chat_id, second_user->id, 1) != 0) {
        logging(ERROR, "Failed to add user '%s' to chat '%s'", second_user->username, chat_name);
        int ret = prepare_simple_response("Failed to add user to chat", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);

        free_user(creator);
        free_user(second_user);
        free(chat_name);

        return ret;
    }


    logging(INFO, "Chat created successfully: '%s' between '%s' and '%s'", chat_name, creator->username, second_user->username);
    int ret = prepare_simple_response("Chat created successfully", STATUS_OK, NULL, context);

    free_user(creator);
    free_user(second_user);
    free(chat_name);

    return ret;

}
