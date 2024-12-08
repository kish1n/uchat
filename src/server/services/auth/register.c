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
        return prepare_simple_response("Failed to parse JSON", STATUS_BAD_REQUEST, NULL, context);
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
        return prepare_simple_response("Missing username or password", STATUS_BAD_REQUEST, parsed_json, context);
    }

    char *passhash = hash_password(password);
    if (!passhash) {
        return prepare_simple_response("Invalid password or login", STATUS_CONFLICT, parsed_json, context);
    }

    int result = create_user(context->db_conn, username, passhash);
    free(passhash);
    json_object_put(parsed_json);

    if (result != 0) {
        return prepare_simple_response("Failed to create user", STATUS_CONFLICT, NULL, context);
    }

    logging(INFO, "Successfully created user: %s", username);

    return prepare_simple_response("User created successfully", STATUS_CREATED, NULL, context);
}
