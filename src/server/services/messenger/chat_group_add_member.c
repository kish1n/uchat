#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../pkg/config/config.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/users/users.h"

int handle_add_member_to_chat(HttpContext *context) {
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_add_member_to_chat");
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
        const char *error_msg = "Invalid JSON";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
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
        const char *error_msg = "Missing or invalid 'chat_id' or 'username'";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    char *sender = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &sender) != 1) {
        logging(ERROR, "JWT verification failed");
        const char *error_msg = create_error_response("unauthorized", STATUS_UNAUTHORIZED);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_UNAUTHORIZED, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    User *user = get_user_by_username(context->db_conn, sender);

    // Check if the user is an admin in the chat
    if (!is_user_in_chat(context->db_conn, chat_id, user->id) || !is_user_admin(context->db_conn, chat_id, user->id)) {

        if (is_user_admin(context->db_conn, chat_id, user->id)) {
            printf("User is an admin\n");
        } else {
            printf("User is not an admin\n");
        }

        logging(ERROR, "User '%s' is not an admin in chat ID '%d'", user->id, chat_id);
        const char *error_msg = create_error_response("forbidden", STATUS_FORBIDDEN);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_FORBIDDEN, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Find the user to be added by username
    User *new_user = get_user_by_username(context->db_conn, username);
    if (!new_user || !new_user->id) {
        logging(ERROR, "User '%s' not found", username);
        const char *error_msg = create_error_response("User not found", STATUS_NOT_FOUND);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Add the user to the chat
    if (add_chat_member(context->db_conn, chat_id, new_user->id, 0) != 0) {
        logging(ERROR, "Failed to add user '%s' to chat ID '%d'", username, chat_id);
        const char *error_msg = create_error_response("Failed to add user to chat", STATUS_INTERNAL_SERVER_ERROR);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        free_user(new_user);
        return ret;
    }

    // Successful response
    const char *success_msg = create_response("User added to chat", STATUS_OK);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    logging(INFO, "User '%s' added to chat ID '%d' by admin '%s'", username, chat_id, user->username);

    json_object_put(parsed_json);
    free(user);
    free(sender);
    free_user(new_user);
    return ret;
}
