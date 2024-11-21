#include "service.h"
#include <stdlib.h>
#include <json-c/json.h>

// Структура для хранения данных тела запроса
typedef struct {
    char *data;
    size_t size;
} RequestBuffer;

// Глобальный буфер (можно улучшить, сделав буфер на основе context)
static RequestBuffer request_buffer = {NULL, 0};

enum MHD_Result handle_request(
    void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **con_cls
) {
    printf("Received request: URL=%s, Method=%s\n", url, method);

    if (strcmp(method, "POST") == 0 && strcmp(url, "/register") == 0) {
        if (*upload_data_size > 0) {
            request_buffer.data = realloc(request_buffer.data, request_buffer.size + *upload_data_size + 1);
            if (!request_buffer.data) {
                fprintf(stderr, "Failed to allocate memory for request buffer\n");
                return MHD_NO;
            }

            memcpy(request_buffer.data + request_buffer.size, upload_data, *upload_data_size);
            request_buffer.size += *upload_data_size;
            request_buffer.data[request_buffer.size] = '\0'; // Завершаем строку

            *upload_data_size = 0; // Указываем, что данные обработаны
            return MHD_YES;
        }

        // Когда upload_data == NULL, все данные получены
        if (upload_data == NULL && request_buffer.size > 0) {
            // Парсим JSON
            struct json_object *request_json = json_tokener_parse(request_buffer.data);
            if (!request_json) {
                const char *response = "{\"error\": \"Invalid JSON\"}";
                struct MHD_Response *http_response = MHD_create_response_from_buffer(
                    strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
                int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, http_response);
                MHD_destroy_response(http_response);
                free(request_buffer.data); // Освобождаем память
                request_buffer.data = NULL;
                request_buffer.size = 0;
                return ret;
            }

            // Ответим тем же JSON, который получили
            const char *response = json_object_to_json_string(request_json);
            struct MHD_Response *http_response = MHD_create_response_from_buffer(
                strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
            int ret = MHD_queue_response(connection, MHD_HTTP_OK, http_response);
            MHD_destroy_response(http_response);

            // Освобождаем ресурсы
            json_object_put(request_json);
            free(request_buffer.data);
            request_buffer.data = NULL;
            request_buffer.size = 0;
            return ret;
        }
    }

    // Обработка неизвестных запросов
    const char *response = "{\"error\": \"Endpoint not found\"}";
    struct MHD_Response *http_response = MHD_create_response_from_buffer(
        strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, http_response);
    MHD_destroy_response(http_response);
    return ret;
}
