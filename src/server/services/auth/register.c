#include "auth_handlers.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../pkg/httputils/httputils.h"
#include "../../db/core/users/users.h"
#include "../../db/core/core.h"
#include "../../pkg/crypto/crypto.h"

int handle_register(HttpContext *context) {
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
        const char *error_msg = create_error_response("Invalid JSON", STATUS_BAD_REQUEST);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    logging(DEBUG, "Parsed JSON: %s", json_object_to_json_string(parsed_json));

    // Extract username and password
    struct json_object *username_obj, *password_obj;
    const char *username = NULL, *password = NULL;

    if (json_object_object_get_ex(parsed_json, "username", &username_obj)) {
        username = json_object_get_string(username_obj);
    }
    if (json_object_object_get_ex(parsed_json, "password", &password_obj)) {
        password = json_object_get_string(password_obj);
    }

    if (!username || !password) {
        const char *error_msg = "Missing username or password";
        logging(ERROR, "Username or password is missing: username=%s, password=%s",
                username ? username : "NULL", password ? password : "NULL");
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    char *passhash = hash_password(password);
    if (!passhash) {
        const char *error_msg = create_response("Failed to hash password", STATUS_CONFLICT);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    int result = create_user(context->db_conn, username, passhash);
    free(passhash);
    json_object_put(parsed_json);

    if (result == 0) {
        const char *success_msg = create_response("User created", STATUS_CREATED);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);

        logging(INFO, "Successfully created user: %s", username);
        return ret;
    } else {
        const char *error_msg = create_error_response("Failed to create user", STATUS_INTERNAL_SERVER_ERROR);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);

        logging(INFO, "Failed to create user: %s", username);
        return ret;
    }
}
