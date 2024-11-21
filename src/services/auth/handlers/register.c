#include "handlers.h"
#include "../../../db/core/users/users.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t size;
} RequestBuffer;

static RequestBuffer buffer = {NULL, 0};

int register_handler(struct MHD_Connection *connection, const char *upload_data, size_t *upload_data_size) {
    printf("Register data %s\n", upload_data);
    if (*upload_data_size > 0) {
        buffer.data = realloc(buffer.data, buffer.size + *upload_data_size + 1);
        if (!buffer.data) {
            return MHD_NO;
        }

        memcpy(buffer.data + buffer.size, upload_data, *upload_data_size);
        buffer.size += *upload_data_size;
        buffer.data[buffer.size] = '\0'; // Завершаем строку

        *upload_data_size = 0; // Указываем, что данные обработаны
        return MHD_YES;        // Ждём, пока данные будут полностью переданы
    }

    if (buffer.size > 0) {
        struct json_object *request_json = json_tokener_parse(buffer.data);
        free(buffer.data);
        buffer.data = NULL;
        buffer.size = 0;

        if (!request_json) {
            const char *response = "{\"error\": \"Invalid JSON\"}";
            struct MHD_Response *http_response = MHD_create_response_from_buffer(
                strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
            int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, http_response);
            MHD_destroy_response(http_response);
            return ret;
        }

        // Проверяем наличие username и password
        struct json_object *username_obj, *password_obj;
        if (!json_object_object_get_ex(request_json, "username", &username_obj) ||
            !json_object_object_get_ex(request_json, "password", &password_obj)) {
            const char *response = "{\"error\": \"Missing username or password\"}";
            struct MHD_Response *http_response = MHD_create_response_from_buffer(
                strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
            int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, http_response);
            MHD_destroy_response(http_response);
            json_object_put(request_json); // Освобождаем JSON-объект
            return ret;
        }

        // Возвращаем тот же JSON
        const char *response = json_object_to_json_string(request_json);
        struct MHD_Response *http_response = MHD_create_response_from_buffer(
            strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, http_response);
        MHD_destroy_response(http_response);

        json_object_put(request_json); // Освобождаем JSON-объект
        return ret;
    }

    const char *response = "{\"error\": \"Empty request body\"}";
    struct MHD_Response *http_response = MHD_create_response_from_buffer(
        strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, http_response);
    MHD_destroy_response(http_response);
    return ret;
}
