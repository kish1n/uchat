#include "auth_handlers.h"
#include "../../../pkg/json_parser/json_parser.h"
#include "../../../pkg/crypto/crypto.h"
#include <json-c/json.h>
#include <stdio.h>
#include <string.h>

// Обработчик регистрации
int handle_register_request(Auth *auth, struct MHD_Connection *connection) {
    // Извлекаем данные из POST-запроса
    const char *username = MHD_lookup_connection_value(connection, MHD_POSTDATA_KIND, "username");
    const char *password = MHD_lookup_connection_value(connection, MHD_POSTDATA_KIND, "password");

    if (!username || !password) {
        // Возвращаем ошибку, если данные не полные
        const char *error_message = "{\"error\": \"Username or password missing\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_message),
                                                                        (void *)error_message,
                                                                        MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Хеширование пароля и генерация соли
    char salt[SALT_SIZE];
    char hashed_password[HASH_SIZE];
    generate_salt(salt, SALT_SIZE);
    hash_password(password, salt, hashed_password);

    // Сохраняем пользователя в базе данных
    const char *query = "INSERT INTO users (username, passhash, salt) VALUES ($1, $2, $3)";
    const char *params[3] = { username, hashed_password, salt };
    PGresult *res = PQexecParams(auth->service->db_connection, query, 3, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        // Ошибка при выполнении запроса
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

    // Успешная регистрация
    const char *success_message = "{\"message\": \"User registered successfully\"}";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(success_message),
                                                                    (void *)success_message,
                                                                    MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}
