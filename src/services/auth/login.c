#include "../../db/core/users/users.h"
#include "../../db/core/core.h"
#include "../../pkg/crypto/crypto.h"
#include "../../pkg/jwt_utils/jwt_utils.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>

#include "auth_handlers.h"

enum MHD_Result handle_login(void *cls,
                                struct MHD_Connection *connection,
                                const char *url,
                                const char *method,
                                const char *version,
                                const char *upload_data,
                                size_t *upload_data_size,
                                void **con_cls,
                                PGconn *db_conn) {
    (void)cls; (void)url; (void)version;

    if (*con_cls == NULL) {
        char *buffer = calloc(1, sizeof(char));
        *con_cls = buffer;
        return MHD_YES;
    }

    char *data = (char *)*con_cls;

    // Читаем данные POST-запроса
    if (*upload_data_size > 0) {
        data = realloc(data, strlen(data) + *upload_data_size + 1);
        strncat(data, upload_data, *upload_data_size);
        *upload_data_size = 0;
        *con_cls = data;
        return MHD_YES;
    }

    // Парсим JSON
    struct json_object *parsed_json = json_tokener_parse(data);
    free(data);
    *con_cls = NULL;

    if (!parsed_json) {
        const char *error_msg = "Invalid JSON";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
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
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    User *user = get_user_by_username(db_conn, username);
    if (!user) {
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Password or login is incorect\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    //hash password
    int cond = verify_password(password, user->passhash);
    if (!cond) {
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Password or login is incorect\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    char *token = generate_jwt(user->uuid, "your_secret_key", 3600); // Генерация токена
    if (!token) {
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Failed to generate token\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        free_user(user);
        json_object_put(parsed_json);
        return ret;
    }

    // Формируем JSON-ответ с токеном
    struct json_object *response_json = json_object_new_object();
    json_object_object_add(response_json, "status", json_object_new_string("success"));
    json_object_object_add(response_json, "token", json_object_new_string(token));

    const char *response_str = json_object_to_json_string(response_json);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_str), (void *)response_str, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    json_object_put(response_json);
    free(token);
    free_user(user);
    json_object_put(parsed_json);
    return ret;
}