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

int handle_update_chat_name(HttpContext *context) {
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_update_chat_name");
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
        return prepare_simple_response("Invalid JSON", STATUS_BAD_REQUEST, NULL, context);
    }

    // Extract required fields from JSON
    struct json_object *chat_id_obj, *new_name_obj;
    int chat_id = -1;
    const char *new_name = NULL;

    if (json_object_object_get_ex(parsed_json, "chat_id", &chat_id_obj)) {
        chat_id = json_object_get_int(chat_id_obj);
    }

    if (json_object_object_get_ex(parsed_json, "new_name", &new_name_obj)) {
        new_name = json_object_get_string(new_name_obj);
    }

    // Validate required fields
    if (chat_id <= 0 || !new_name || strlen(new_name) == 0) {
        return prepare_simple_response("Missing or invalid 'chat_id' or 'new_name'", STATUS_BAD_REQUEST, parsed_json, context);
    }

    // Extract user from JWT
    char *username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &username) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_simple_response("Invalid JWT", STATUS_UNAUTHORIZED, parsed_json, context);
    }

    // Check if user is part of the chat
    User *user = get_user_by_username(context->db_conn, username);
    if (!user || !is_user_in_chat(context->db_conn, chat_id, user->id)) {
        logging(ERROR, "User '%s' is not part of chat ID '%d'", username, chat_id);
        int ret = prepare_simple_response("User is not in the chat", STATUS_FORBIDDEN, parsed_json, context);

        free(username);
        return ret;
    }

    // Check if chat is a group
    if (!is_chat_group(context->db_conn, chat_id)) {
        logging(ERROR, "Chat ID '%d' is not a group", chat_id);
        int ret =prepare_simple_response("Chat is not a group", STATUS_FORBIDDEN, parsed_json, context);

        free(username);
        return ret;
    }

    // Update chat name in database
    if (update_chat_name(context->db_conn, chat_id, new_name) != 0) {
        logging(ERROR, "Failed to update chat name for chat ID '%d'", chat_id);
        int ret = prepare_simple_response("Failed to update chat name", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);

        free(username);
        return ret;
    }

    logging(INFO, "User '%s' updated chat ID '%d' to name '%s'", username, chat_id, new_name);
    int ret = prepare_simple_response("Chat name updated success", STATUS_OK, parsed_json, context);

    free(username);

    return ret;
}
