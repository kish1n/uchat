#include "auth_handlers.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../pkg/httputils/httputils.h"
#include "../../db/core/users/users.h"
#include "../../db/core/core.h"
#include "../../pkg/crypto/crypto.h"
#include "../../pkg/jwt_utils/jwt_utils.h"

int handle_update_username(HttpContext *context) {
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
        return prepare_response("Invalid JSON", STATUS_BAD_REQUEST, parsed_json, context);
    }

    struct json_object *new_username_obj, *password_obj;
    const char *new_username = NULL;
    const char *password = NULL;

    if (json_object_object_get_ex(parsed_json, "new_username", &new_username_obj)) {
        new_username = json_object_get_string(new_username_obj);
    }
    if (json_object_object_get_ex(parsed_json, "password", &password_obj)) {
        password = json_object_get_string(password_obj);
    }

    if (!new_username || strlen(new_username) == 0 || !password) {
        logging(ERROR, "Missing or empty new_username or password in request");
        return prepare_response("Invalid JSON", STATUS_BAD_REQUEST, parsed_json, context);
    }
    logging(DEBUG, "New username: %s, Password: %s", new_username, password);

    char *user_id = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, cfg.security.jwt_secret, &user_id) != 1) {
        logging(ERROR, "JWT verification failed");
        return prepare_response("Invalid JWT", STATUS_UNAUTHORIZED, parsed_json, context);
    }

    logging(DEBUG, "User ID: %s", user_id);

    if (!check_user_credentials(context->db_conn, user_id, password)) {
        logging(ERROR, "Invalid password for user ID: %s", user_id);
        int ret = prepare_response("Invalid password", STATUS_UNAUTHORIZED, parsed_json, context);
        free(user_id);
        return ret;
    }

    logging(DEBUG, "Password verified for user ID: %s", user_id);

    if (get_user_by_username(context->db_conn, new_username)) {
        logging(ERROR, "Username already taken: %s", new_username);
        int ret = prepare_response("Username already taken", STATUS_CONFLICT, parsed_json, context);
        free(user_id);
        return ret;
    }

    logging(DEBUG, "Username available: %s", new_username);

    if (update_user_username(context->db_conn, user_id, new_username) != 0) {
        logging(ERROR, "Failed to update username for user ID: %s", user_id);
        int ret = prepare_response("Failed to update username", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);
        free(user_id);
        return ret;
    }

    char *new_token = generate_jwt(new_username, cfg.security.jwt_secret, 3600);
    if (!new_token) {
        logging(ERROR, "Failed to generate new JWT for user: %s", new_username);
        int ret = prepare_response("Failed to generate new JWT", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);
        free(user_id);
        return ret;
    }

    struct json_object *response_json = json_object_new_object();
    json_object_object_add(response_json, "status", json_object_new_string("success"));
    json_object_object_add(response_json, "token", json_object_new_string(new_token));

    int ret = prepare_response("Username updated successfully", STATUS_OK, response_json, context);

    logging(INFO, "User %s successfully updated username to %s and received new token", user_id, new_username);

    json_object_put(parsed_json);
    free(user_id);
    free(new_token);

    return ret;
}
