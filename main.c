#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h> // Для работы с HTTP
#include <json-c/json.h> // Для работы с JSON

#define PORT 8080

struct RequestData {
    char *data;
    size_t size;
};

void free_request_data(void *con_cls) {
    struct RequestData *request_data = (struct RequestData *)con_cls;
    if (request_data) {
        if (request_data->data) free(request_data->data);
        free(request_data);
    }
}

static int handle_request(void *cls,
                          struct MHD_Connection *connection,
                          const char *url,
                          const char *method,
                          const char *version,
                          const char *upload_data,
                          size_t *upload_data_size,
                          void **con_cls) {
    (void)cls; (void)url; (void)version;

    // Инициализация структуры данных для каждого соединения
    if (*con_cls == NULL) {
        struct RequestData *request_data = calloc(1, sizeof(struct RequestData));
        if (!request_data) return MHD_NO; // Ошибка выделения памяти
        *con_cls = request_data;
        return MHD_YES;
    }

    struct RequestData *request_data = (struct RequestData *)*con_cls;

    // Обрабатываем только POST запросы
    if (strcmp(method, "POST") != 0) {
        const char *error_msg = "Only POST method is supported";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Читаем данные из запроса
    if (*upload_data_size > 0) {
        request_data->data = realloc(request_data->data, request_data->size + *upload_data_size + 1);
        if (!request_data->data) return MHD_NO; // Ошибка выделения памяти
        memcpy(request_data->data + request_data->size, upload_data, *upload_data_size);
        request_data->size += *upload_data_size;
        request_data->data[request_data->size] = '\0'; // Завершающий символ строки
        *upload_data_size = 0;
        return MHD_YES; // Продолжаем обработку
    }

    // Если запрос завершён, разбираем JSON
    struct json_object *parsed_json = json_tokener_parse(request_data->data);
    if (!parsed_json) {
        const char *error_msg = "Invalid JSON";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        free_request_data(request_data);
        return ret;
    }

    // Формируем ответ с тем же JSON
    const char *response_data = json_object_to_json_string_ext(parsed_json, JSON_C_TO_STRING_PLAIN);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_data), (void *)response_data, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    // Очистка памяти
    json_object_put(parsed_json);
    free_request_data(request_data);
    *con_cls = NULL;

    return ret;
}

int main() {
    struct MHD_Daemon *server;

    server = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
        &handle_request, NULL, MHD_OPTION_NOTIFY_COMPLETED, &free_request_data, NULL, MHD_OPTION_END);
    if (!server) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server is running on port %d\n", PORT);

    getchar(); // Ожидание ввода для остановки сервера

    MHD_stop_daemon(server);
    return 0;
}
