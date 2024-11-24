#include "../../db/core/users/users.h"
#include "../../db/core/core.h"
#include "../../pkg/crypto/crypto.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>

#include "auth_handlers.h"

enum MHD_Result handle_register(HttpContext *context) {
    if (!context) {
        logging(ERROR, "Invalid context passed to handle_register");
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

    struct json_object *username_obj, *password_obj;
    const char *username = NULL;
    const char *password = NULL;

    if (json_object_object_get_ex(parsed_json, "username", &username_obj) &&
        json_object_object_get_ex(parsed_json, "password", &password_obj)) {
        username = json_object_get_string(username_obj);
        password = json_object_get_string(password_obj);
    }

    if (!username || !password) {
        const char *error_msg = "Missing username or password";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);

        logging(INFO, "Failed to register user: missing username or password");
        return ret;
    }

    char *passhash = hash_password(password);
    if (!passhash) {
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Password hashing failed\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    int result = create_user(context->db_conn, username, passhash);
    free(passhash);  // Освобождаем память для passhash
    json_object_put(parsed_json);

    if (result == 0) {
        const char *success_msg = "{\"status\":\"success\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);

        logging(INFO, "Successfully created user: %s", username);
        return ret;
    } else {
        const char *error_msg = "{\"status\":\"error\",\"message\":\"User creation failed\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);

        logging(INFO, "Failed to create user: %s", username);
        return ret;
    }
}
