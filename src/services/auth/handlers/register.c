#include "auth_handlers.h"
#include "../../../pkg/json_parser/json_parser.h"
#include "../../../pkg/crypto/crypto.h"
#include <json-c/json.h>
#include <stdio.h>
#include <string.h>
#include <microhttpd.h>
#include <libpq-fe.h>

enum MHD_Result handle_register_request(Auth *auth, struct MHD_Connection *connection,
                                        const char *url, const char *method,
                                        const char *upload_data, size_t *upload_data_size, void **con_cls) {

    static char body[8192];
    if (*upload_data_size != 0) {
        strncat(body, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }

    struct json_object *json_body = json_tokener_parse(body);
    if (!json_body) {
        const char *error_message = "{\"error\": \"Invalid JSON format\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_message),
                                                                        (void *)error_message,
                                                                        MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/json");
        enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    struct json_object *username_obj, *password_obj;
    const char *username = NULL, *password = NULL;

    if (json_object_object_get_ex(json_body, "username", &username_obj)) {
        username = json_object_get_string(username_obj);
    }
    if (json_object_object_get_ex(json_body, "password", &password_obj)) {
        password = json_object_get_string(password_obj);
    }

    if (!username || !password) {
        const char *error_message = "{\"error\": \"Username or password missing\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_message),
                                                                        (void *)error_message,
                                                                        MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/json");
        enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(json_body);
        return ret;
    }

    printf("Username: %s, Password: %s\n", username, password);

    char salt[SALT_SIZE];
    char hashed_password[HASH_SIZE];
    generate_salt(salt, SALT_SIZE);
    hash_password(password, salt, hashed_password);

    const char *query = "INSERT INTO users (username, passhash, salt) VALUES ($1, $2, $3)";
    const char *params[3] = { username, hashed_password, salt };
    PGresult *res = PQexecParams(auth->service->db_connection, query, 3, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        const char *error_message = "{\"error\": \"Registration failed. User might already exist.\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_message),
                                                                        (void *)error_message,
                                                                        MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        PQclear(res);
        return ret;
    }
    PQclear(res);

    const char *success_message = "{\"message\": \"User registered successfully\"}";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(success_message),
                                                                    (void *)success_message,
                                                                    MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return MHD_YES;
}
