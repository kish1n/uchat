#include "auth_handlers.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../db/core/users/users.h"
#include "../../pkg/httputils/httputils.h"
#include "../../pkg/jwt_utils/jwt_utils.h"

int handle_login(HttpContext *context) {
    logging(DEBUG, "Enter in handle login");
    Config cfg;
    load_config("config.yaml", &cfg);
    logging(DEBUG, "After loading config");

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_create_chat");
        return MHD_NO;
    }
    logging(DEBUG, "After checking context");

    if (*context->con_cls == NULL) {
        char *buffer = calloc(1, sizeof(char));
        *context->con_cls = buffer;
        return MHD_YES;
    }
    logging(DEBUG, "After checking con_cls");

    char *data = (char *)*context->con_cls;
    logging(DEBUG, "Before checking upload_data_size");
    if (*context->upload_data_size > 0) {
        logging(DEBUG, "Upload_data_size >0");
        data = realloc(data, strlen(data) + *context->upload_data_size + 1);
        logging(DEBUG, "After realloc");
        strncat(data, context->upload_data, *context->upload_data_size);
        
        *context->upload_data_size = 0;
        *context->con_cls = data;
        logging(DEBUG, "Before MHD_YES");
        return MHD_YES;
    }
    logging(DEBUG, "After checking upload_data_size");

    struct json_object *parsed_json = json_tokener_parse(data);
    free(data);
    *context->con_cls = NULL;

    logging(DEBUG, "Data was gotten");

    if (!parsed_json) {
        return prepare_simple_response("Failed to parse JSON", STATUS_BAD_REQUEST, NULL, context);
    }

    // Extract username and password
    struct json_object *username_obj, *password_obj;
    const char *username = NULL, *password = NULL;

    if (json_object_object_get_ex(parsed_json, "username", &username_obj)) {
        username = json_object_get_string(username_obj);
    }
    if (json_object_object_get_ex(parsed_json, "password", &password_obj)) {
        password = json_object_get_string(password_obj);
    }
    logging(DEBUG, "Get username and password");
    if (!username || !password) {
        return prepare_simple_response("Missing 'username' or 'password' in request", STATUS_BAD_REQUEST,  parsed_json, context);
    }

    logging(DEBUG, "Before checking user credentials");

    // Check user credentials (pseudo code for database verification)
    if (!check_user_credentials(context->db_conn, username, password)) {
        return prepare_simple_response("Invalid username or password", STATUS_UNAUTHORIZED, parsed_json, context);
    }

    logging(DEBUG, "After checking user credentials");

    // Generate JWT token
    char *token = generate_jwt(username, cfg.security.jwt_secret, 3600);
    if (!token) {
        return prepare_simple_response("Failed to generate JWT token", STATUS_INTERNAL_SERVER_ERROR, parsed_json, context);
    }
    logging(DEBUG, "Token was generated");

    // Create success response
    struct json_object *response_json = json_object_new_object();
    json_object_object_add(response_json, "status", json_object_new_string("success"));
    json_object_object_add(response_json, "token", json_object_new_string(token));

    const char *response_str = json_object_to_json_string(response_json);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_str), (void *)response_str, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    json_object_put(response_json);
    json_object_put(parsed_json);
    free(token);

    logging(INFO, "User %s authenticated successfully", username);
    return ret;
}
