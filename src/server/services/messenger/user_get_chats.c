#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../service.h"
#include "../../pkg/httputils/httputils.h"
#include "../../pkg/http_response/response.h"
#include "../../db/core/messages/messages.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../db/core/users/users.h"

int handle_get_user_chats(HttpContext *context) {
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_get_chat_history");
        return MHD_NO;
    }

    char *username = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &username) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_simple_response("Invalid JWT", STATUS_UNAUTHORIZED, NULL, context);
    }

    User *sender = get_user_by_username(context->db_conn, username);
    free(username);

    if (!sender) {
        logging(ERROR, "User not found: %s", username);
        return prepare_simple_response("User not found", STATUS_NOT_FOUND, NULL, context);
    }

    char *chats_json = get_user_chats(context->db_conn, sender->id);
    if (!chats_json) {
        logging(INFO, "Failed to get user chats");
        return prepare_simple_response("Failed to get user chats", STATUS_INTERNAL_SERVER_ERROR, NULL, context);
    }

    logging(INFO, "User chats gets successfully");
    return prepare_response( STATUS_OK, json_tokener_parse(chats_json), context);
}