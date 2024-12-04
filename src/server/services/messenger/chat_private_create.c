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
        const char *error_msg = "Invalid JSON";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    struct json_object *name_obj, *users_array;
    const char *with_user = NULL;

    //LOGIC:

    if (json_object_object_get_ex(parsed_json, "with_user", &name_obj)) {
        with_user = json_object_get_string(name_obj);
    }

    char *creator_id = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &creator_id) != 1) {
        logging(ERROR, "JWT verification failed");
        const char *error_msg = create_error_response("unauthorized", STATUS_UNAUTHORIZED);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_UNAUTHORIZED, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    User *creator = get_user_by_uuid(context->db_conn, creator_id);
    User *second_user = get_user_by_username(context->db_conn, with_user);

    free(creator_id);
    if (!second_user->id) {
        logging(ERROR, "User with username '%s' not found", with_user);
        const char *error_msg = create_error_response("User not found", STATUS_NOT_FOUND);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        free(creator_id);
        return ret;
    }

    int chat_id = private_chat_exist(context->db_conn, creator->id, second_user->id);
    if (chat_id == -1) {
        logging(ERROR, "Failed to check if private chat exists");
        const char *error_msg = create_error_response("Failed to check if private chat exists", STATUS_INTERNAL_SERVER_ERROR);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    } else if (chat_id > 0) {
        const char *error_msg = create_response("already exists", STATUS_CONFLICT);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_CONFLICT, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    char *chat_name = malloc(strlen(with_user) + strlen(second_user->username) + 2);
    chat_id = create_chat(context->db_conn, chat_name, 0);

    if (chat_id <= 0) {
        const char *error_msg = "Failed to create chat";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);

        logging(ERROR, "Failed to create chat '%s'", chat_name);
        return ret;
    }

    if (add_chat_member(context->db_conn, chat_id, creator->id, 1) != 0) {
        const char *error_msg = "Failed to add chat creator";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);

        logging(ERROR, "Failed to add creator '%s' to chat with user '%s'", creator->username, second_user->username);
        return ret;
    }
    if (add_chat_member(context->db_conn, chat_id, second_user->id, 1) != 0) {
        const char *error_msg = "Failed to add chat creator";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);

        logging(ERROR, "Failed to add user '%s' to chat with creator '%s'",  creator->username, second_user->username);
        return ret;
    }
    json_object_put(parsed_json);

    const char *success_msg = create_response("Success", STATUS_OK);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    logging(INFO, "Success chat crete users: '%s' and '%s'", creator->username, second_user->username);
    return ret;
}
