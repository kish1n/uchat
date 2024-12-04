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
        const char *error_msg = "Invalid JSON";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    struct json_object *name_obj, *users_array;
    const char *name = NULL;

    if (json_object_object_get_ex(parsed_json, "name", &name_obj)) {
        name = json_object_get_string(name_obj);
    }
    if (!json_object_object_get_ex(parsed_json, "users", &users_array)) {
        const char *error_msg = "Missing 'users' array";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    if (!name || strlen(name) == 0) {
        const char *error_msg = "Missing or invalid 'name' field";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    int chat_id = create_chat(context->db_conn, name, 1);
    if (chat_id <= 0) {
        const char *error_msg = "Failed to create chat";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);

        logging(ERROR, "Failed to create chat '%s'", name);
        return ret;
    }

    char *creator_username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &creator_username) != 1) {
        logging(ERROR, "JWT verification failed");
        const char *error_msg = create_error_response("unauthorized", STATUS_UNAUTHORIZED);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_UNAUTHORIZED, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    User *creator = get_user_by_username(context->db_conn, creator_username);

    if (add_chat_member(context->db_conn, chat_id, creator->id, 1) != 0) {
        const char *error_msg = "Failed to add chat creator";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);

        logging(ERROR, "Failed to add creator '%s' to chat '%s'", creator->username, name);
        return ret;
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

    json_object_put(parsed_json);
    free(creator_username);

    const char *success_msg = create_response("success", STATUS_OK);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    logging(INFO, "Group chat '%s' created successfully with ID %d", name, chat_id);
    return ret;
}
