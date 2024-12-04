#include "../../db/core/users/users.h"
#include "../../db/core/core.h"
#include "../../pkg/crypto/crypto.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>

#include "auth_handlers.h"

int handle_update_username(HttpContext *context) {
    /*Config cfg;
    load_config("config.yaml", &cfg);*/

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_update_username");
        return MHD_NO;
    }

    if (*context->con_cls == NULL) {
        char *buffer = calloc(1, sizeof(char));
        if (!buffer) {
            logging(ERROR, "Memory allocation failed for con_cls");
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
            logging(ERROR, "Memory allocation failed during data accumulation");
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
        logging(ERROR, "Invalid JSON format in request");
        const char *error_msg = create_error_response("Invalid JSON", STATUS_BAD_REQUEST);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
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
        const char *error_msg = create_error_response("Invalid JSON", STATUS_BAD_REQUEST);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }
    logging(DEBUG, "New username: %s, Password: %s", new_username, password);

    char *user_id = NULL;
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt || verify_jwt(jwt, context->jwt_secret, &user_id) != 1) {
        logging(ERROR, "JWT verification failed");
        const char *error_msg = create_error_response("unauthorized", STATUS_UNAUTHORIZED);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_UNAUTHORIZED, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    logging(DEBUG, "User ID: %s", user_id);

    if (!check_user_credentials(context->db_conn, user_id, password)) {
        logging(ERROR, "Invalid password for user ID: %s", user_id);
        const char *error_msg = create_error_response("Invalid password", STATUS_FORBIDDEN);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_UNAUTHORIZED, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        free(user_id);
        return ret;
    }

    logging(DEBUG, "Password verified for user ID: %s", user_id);

    if (get_user_by_username(context->db_conn, new_username)) {
        logging(ERROR, "Username already taken: %s", new_username);
        const char *error_msg = create_error_response("Username already taken", STATUS_CONFLICT);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_CONFLICT, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        free(user_id);
        return ret;
    }

    logging(DEBUG, "Username available: %s", new_username);

    if (update_user_username(context->db_conn, user_id, new_username) != 0) {
        logging(ERROR, "Failed to update username for user ID: %s", user_id);
        const char *error_msg = create_error_response("Failed to update username", STATUS_INTERNAL_SERVER_ERROR);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        free(user_id);
        return ret;
    }

    char *new_token = generate_jwt(new_username, context->jwt_secret, 3600);
    if (!new_token) {
        logging(ERROR, "Failed to generate new JWT for user: %s", new_username);
        const char *error_msg = create_error_response("Failed to generate token", STATUS_INTERNAL_SERVER_ERROR);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        free(user_id);
        return ret;
    }

    struct json_object *response_json = json_object_new_object();
    json_object_object_add(response_json, "status", json_object_new_string("success"));
    json_object_object_add(response_json, "token", json_object_new_string(new_token));

    const char *response_str = json_object_to_json_string(response_json);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_str), (void *)response_str, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    logging(INFO, "User %s successfully updated username to %s and received new token", user_id, new_username);

    json_object_put(response_json);
    json_object_put(parsed_json);
    free(user_id);
    free(new_token);

    return ret;
}
