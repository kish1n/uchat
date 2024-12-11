#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../service.h"
#include "../../pkg/config/config.h"
#include "../../pkg/httputils/httputils.h"
#include "../../pkg/http_response/response.h"
#include "../../db/core/messages/messages.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../db/core/users/users.h"

int handle_edit_message(HttpContext *context) {
    Config cfg;
    if (load_config("config.yaml", &cfg) != 0) {
        logging(ERROR, "Failed to load config");
        return MHD_NO;
    }

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_edit_message");
        return MHD_NO;
    }

    if (*context->con_cls == NULL) {
        char *buffer = calloc(1, sizeof(char));
        if (!buffer) {
            logging(ERROR, "Memory allocation failed");
            return MHD_NO;
        }
        *context->con_cls = buffer;
        return MHD_YES;
    }

    char *data = (char *)*context->con_cls;

    if (*context->upload_data_size > 0) {
        size_t new_size = strlen(data) + *context->upload_data_size + 1;
        char *temp = realloc(data, new_size);
        if (!temp) {
            logging(ERROR, "Memory reallocation failed");
            free(data);
            *context->con_cls = NULL;
            return MHD_NO;
        }
        data = temp;
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


    struct json_object *message_id_obj, *new_content_obj;
    int message_id = -1;
    const char *new_content = NULL;

    if (json_object_object_get_ex(parsed_json, "message_id", &message_id_obj)) {
        message_id = json_object_get_int(message_id_obj);
    }

    if (json_object_object_get_ex(parsed_json, "new_content", &new_content_obj)) {
        new_content = json_object_get_string(new_content_obj);
    }

    if (message_id <= 0 || !new_content || strlen(new_content) == 0) {
        logging(ERROR, "Invalid fields in request");
        return prepare_simple_response("Invalid fields in request", STATUS_BAD_REQUEST, parsed_json, context);
    }

    char *username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &username) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_simple_response("Invalid JWT", STATUS_UNAUTHORIZED, parsed_json, context);
    }

    User *user = get_user_by_username(context->db_conn, username);
    free(username);
    if (!user) {
        logging(ERROR, "User not found");
        return prepare_simple_response("User not found", STATUS_NOT_FOUND, parsed_json, context);
    }

    Message message;
    if (get_message_by_id(context->db_conn, message_id, &message, &cfg) != 0) {
        logging(ERROR, "Message not found");
        int ret = prepare_simple_response("Message not found", STATUS_NOT_FOUND, parsed_json, context);

        free_user(user);
        return ret;
    }

    if (!is_user_in_chat(context->db_conn, message.chat_id, user->id)) {
        logging(ERROR, "User not in chat");
        int ret = prepare_simple_response("User not in chat", STATUS_FORBIDDEN, parsed_json, context);

        free_user(user);
        return ret;
    }

    if (strcmp(message.sender_id, user->id) != 0) {
        logging(ERROR, "User not message owner");
        int ret = prepare_simple_response("User not message owner", STATUS_FORBIDDEN, parsed_json, context);

        free_user(user);
        return ret;
    }

    if (edit_message(context->db_conn, message_id, new_content, &cfg) != 0) {
        logging(ERROR, "Failed to edit message");
        int ret = prepare_simple_response("Failed to edit message", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);

        free_user(user);
        return ret;
    }

    logging(INFO, "Message edited successfully");
    return prepare_simple_response("Message edited successfully", STATUS_OK, parsed_json, context);
}
