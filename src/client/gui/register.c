#include "client.h"

// Функция для отправки POST-запроса
void send_register_request(const char *url, const char *username, const char *password) {
    GError *error = NULL;
    GSocketClient *client = g_socket_client_new();
    GSocketConnection *connection = NULL;
    GOutputStream *output_stream = NULL;
    GInputStream *input_stream = NULL;
    cJSON *json = NULL;
    char *json_data = NULL;
    char *request = NULL;

    // Создание JSON-строки
    json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "username", username);
    cJSON_AddStringToObject(json, "password", password);
    json_data = cJSON_PrintUnformatted(json);

    // Парсинг URL
    GUri *uri = g_uri_parse(url, G_URI_FLAGS_NONE, &error);
    if (!uri) {
        g_printerr("Ошибка парсинга URL: %s\n", error->message);
        goto cleanup;
    }

    // Установка соединения
    connection = g_socket_client_connect_to_host(client, g_uri_get_host(uri), g_uri_get_port(uri), NULL, &error);
    if (!connection) {
        g_printerr("Ошибка подключения: %s\n", error->message);
        goto cleanup;
    }

    output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    input_stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    if (!output_stream || !input_stream) {
        g_printerr("Ошибка получения потоков ввода/вывода.\n");
        goto cleanup;
    }

    // Формирование HTTP-запроса
    size_t request_size = strlen(json_data) + 200;
    request = g_malloc0(request_size);
    snprintf(request, request_size,
             "POST %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             g_uri_get_path(uri), g_uri_get_host(uri), strlen(json_data), json_data);

    // Отправка запроса
    g_output_stream_write_all(output_stream, request, strlen(request), NULL, NULL, &error);
    if (error) {
        g_printerr("Ошибка отправки данных: %s\n", error->message);
        goto cleanup;
    } else {
        g_print("Запрос отправлен успешно.\n");
    }

    // Чтение ответа
    char buffer[1024];
    GString *response = g_string_new(NULL);

    while (TRUE) {
        gssize bytes_read = g_input_stream_read(input_stream, buffer, sizeof(buffer) - 1, NULL, &error);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            g_string_append(response, buffer);
        } else if (bytes_read == 0) {
            break;
        } else {
            g_printerr("Ошибка чтения ответа: %s\n", error->message);
            g_clear_error(&error);
            break;
        }
    }

    if (response->len > 0) {
        g_print("Ответ сервера:\n%s\n", response->str);
    }

    g_string_free(response, TRUE);

cleanup:
    if (json) cJSON_Delete(json);
    if (json_data) g_free(json_data);
    if (request) g_free(request);
    if (uri) g_object_unref(uri);
    if (client) g_object_unref(client);
    if (connection) g_object_unref(connection);
    if (error) g_clear_error(&error);
}

