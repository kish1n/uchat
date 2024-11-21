// #include "./auth/auth_handlers.h"
// #include "../db/core/users/users.h"
// #include "../db/core/core.h"
// #include <json-c/json.h>
// #include <stdlib.h>
// #include <string.h>
//
// enum MHD_Result handle_register(void *cls,
//     struct MHD_Connection *connection,
//     const char *url,
//     const char *method,
//     const char *version,
//     const char *upload_data,
//     size_t *upload_data_size,
//     void **con_cls) {
//     (void)cls; (void)url; (void)version;
//
//     if (*con_cls == NULL) {
//         char *buffer = calloc(1, sizeof(char));
//         *con_cls = buffer;
//         return MHD_YES;
//     }
//
//     char *data = (char *)*con_cls;
//
//     // Читаем данные POST-запроса
//     if (*upload_data_size > 0) {
//         data = realloc(data, strlen(data) + *upload_data_size + 1);
//         strncat(data, upload_data, *upload_data_size);
//         *upload_data_size = 0;
//         *con_cls = data;
//         return MHD_YES;
//     }
//
//     // Парсим JSON
//     struct json_object *parsed_json = json_tokener_parse(data);
//     free(data);
//     *con_cls = NULL;
//
//     if (!parsed_json) {
//         const char *error_msg = "Invalid JSON";
//         struct MHD_Response *response = MHD_create_response_from_buffer(
//             strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
//         int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
//         MHD_destroy_response(response);
//         return ret;
//     }
//
//     struct json_object *username_obj, *password_obj;
//     const char *username = NULL;
//     const char *password = NULL;
//
//     if (json_object_object_get_ex(parsed_json, "username", &username_obj) &&
//         json_object_object_get_ex(parsed_json, "password", &password_obj)) {
//         username = json_object_get_string(username_obj);
//         password = json_object_get_string(password_obj);
//     }
//
//     if (!username || !password) {
//         const char *error_msg = "Missing username or password";
//         struct MHD_Response *response = MHD_create_response_from_buffer(
//             strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
//         int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
//         MHD_destroy_response(response);
//         json_object_put(parsed_json);
//         return ret;
//     }
//
//     // Хэширование пароля (упрощённо)
//     char passhash[256];
//     snprintf(passhash, sizeof(passhash), "hash_%s", password); // Замените на реальное хэширование
//
//     PGconn *db_conn = (PGconn *)cls;
//     int result = create_user(db_conn, username, passhash);
//
//     json_object_put(parsed_json);
//
//     if (result == 0) {
//         const char *success_msg = "{\"status\":\"success\"}";
//         struct MHD_Response *response = MHD_create_response_from_buffer(
//             strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
//         int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
//         MHD_destroy_response(response);
//         return ret;
//     } else {
//         const char *error_msg = "{\"status\":\"error\",\"message\":\"User creation failed\"}";
//         struct MHD_Response *response = MHD_create_response_from_buffer(
//             strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
//         int ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
//         MHD_destroy_response(response);
//         return ret;
//     }
// }
//
