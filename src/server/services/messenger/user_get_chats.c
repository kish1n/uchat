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
        const char *error_msg = create_error_response("unauthorized", STATUS_UNAUTHORIZED);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_UNAUTHORIZED, response);
        MHD_destroy_response(response);
        return ret;
    }

    User *sender = get_user_by_username(context->db_conn, username);
    free(username);

    if (!sender) {
        logging(ERROR, "User not found: %s", username);
        const char *error_msg = create_error_response("User not found", STATUS_NOT_FOUND);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret;
    }

    char *chats_json = get_user_chats(context->db_conn, sender->id);
    if (!chats_json) {
        logging(INFO, "Failed to get user chats");
        const char *error_msg = create_error_response("No chats found for user", STATUS_NOT_FOUND);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        return MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
    }

    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(chats_json), (void *)chats_json, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
    free(chats_json);
    MHD_destroy_response(response);
    logging(INFO, "User chats gets successfully");
    return ret;
}