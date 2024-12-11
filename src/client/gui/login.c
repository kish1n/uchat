#include "client.h"
#include <string.h>

char* global_token = NULL;
char* global_pass = NULL;
char* username_global;

void set_global_token(const char *token) {
    // Перевірка та звільнення попередньої пам'яті, якщо вона вже виділена
    if (global_token != NULL) {
        free(global_token);
    }

    // Копіюємо нове значення
    global_token = strdup(token);
    if (global_token == NULL) {
        fprintf(stderr, "Помилка виділення пам'яті для global_token.\n");
        exit(EXIT_FAILURE);
    }
}

const char *get_global_token() {
    if (global_token == NULL) {
        fprintf(stderr, "Попередження: global_token ще не встановлений.\n");
        return NULL;
    }
    return global_token;
}

void set_pass(const char *pass) {
    // Перевірка та звільнення попередньої пам'яті, якщо вона вже виділена
    if (global_pass != NULL) {
        free(global_pass);
    }

    // Копіюємо нове значення
    global_pass = strdup(pass);
    if (global_pass == NULL) {
        fprintf(stderr, "Помилка виділення пам'яті для global_token.\n");
        exit(EXIT_FAILURE);
    }
}

const char *get_pass() {
    if (global_pass == NULL) {
        fprintf(stderr, "Попередження: global_token ще не встановлений.\n");
        return NULL;
    }
    return global_pass;
}

char *extract_json_from_response(const char *response_str) {
    // Знайти роздільник між заголовками та тілом (\r\n\r\n)
    const char *body = strstr(response_str, "\r\n\r\n");
    if (body) {
        body += 4; // Перейти до початку JSON (пропустити \r\n\r\n)
        // Повернути копію JSON-частини
        return g_strdup(body);
    }
    g_printerr("Помилка: тіло відповіді не знайдено.\n");
    return NULL;
}

// Сеттер для глобального юзернейму
void set_username_global(const char *username) {
    // Звільняємо попередню пам'ять, якщо вона була виділена
    if (username_global != NULL) {
        free(username_global);
        username_global = NULL; // Очищаємо вказівник
    }

    // Виділяємо нову пам'ять для юзернейму
    username_global = strdup(username);
    if (username_global == NULL) {
        fprintf(stderr, "Помилка виділення пам'яті для username_global.\n");
        exit(EXIT_FAILURE);
    }
}

// Гетер для глобального юзернейму
const char *get_username_global() {
    if (username_global == NULL) {
        fprintf(stderr, "Попередження: username_global ще не встановлений.\n");
        return NULL;
    }
    return username_global;
}
// Функція для відправки запиту на авторизацію
void send_login_request(const char *url, const char *username, const char *password) {
    GError *error = NULL;
    GSocketClient *client = g_socket_client_new();
    GSocketConnection *connection = NULL;
    GOutputStream *output_stream = NULL;
    GInputStream *input_stream = NULL;
    cJSON *json = NULL;
    char *json_data = NULL;
    char *request = NULL;

    // Створення JSON-об'єкта
    json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "username", username);
    cJSON_AddStringToObject(json, "password", password);
    json_data = cJSON_PrintUnformatted(json);

    // Парсинг URL
    GUri *uri = g_uri_parse(url, G_URI_FLAGS_NONE, &error);
    if (!uri) {
        g_printerr("Помилка парсингу URL: %s\n", error->message);
        goto cleanup;
    }

    // Встановлення з'єднання
    connection = g_socket_client_connect_to_host(client, g_uri_get_host(uri), g_uri_get_port(uri), NULL, &error);
    if (!connection) {
        g_printerr("Помилка підключення: %s\n", error->message);
        goto cleanup;
    }

    output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    input_stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    if (!output_stream || !input_stream) {
        g_printerr("Помилка отримання потоків вводу/виводу.\n");
        goto cleanup;
    }

    // Формування HTTP-запиту
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

    // Відправка запиту
    g_output_stream_write_all(output_stream, request, strlen(request), NULL, NULL, &error);
    if (error) {
        g_printerr("Помилка відправки даних: %s\n", error->message);
        goto cleanup;
    } else {
        g_print("Запит відправлено успішно.\n");
    }

    // Читання відповіді
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
            g_printerr("Помилка читання відповіді: %s\n", error->message);
            g_clear_error(&error);
            break;
        }
    }

    if (response->len > 0) {
        g_print("Відповідь сервера:\n%s\n", response->str);
    }
    //check status
    cJSON *json_response = cJSON_Parse(extract_json_from_response(response->str));
    if (json_response == NULL) {
        g_printerr("Помилка парсингу відповіді сервера.\n");
    } else {
        // Отримати поле "status"
        cJSON *status = cJSON_GetObjectItemCaseSensitive(json_response, "status");
        cJSON *token = cJSON_GetObjectItemCaseSensitive(json_response, "token");
        if (cJSON_IsString(token) && (token->valuestring != NULL)) {

            set_global_token(token->valuestring);
            set_pass(password);


        }
        
        if (cJSON_IsString(status) && (status->valuestring != NULL)) {

            // Перевірити статус
            if (strcmp(status->valuestring, "success") == 0) {
                g_print("Авторизація успішна.\n");
                

                set_username_global(username);
                char *norm_url = build_url(host, port, "user/chats");
                fetch_and_load_chats(norm_url);
                gtk_widget_show_all(window_main);
                gtk_widget_hide(auth_dialog);
            } else {
                g_print("Авторизація не вдалася: %s\n", status->valuestring);
            }
        }
        
        

        // Отримати повідомлення, якщо є
        cJSON *message = cJSON_GetObjectItemCaseSensitive(json_response, "message");
        if (cJSON_IsString(message) && (message->valuestring != NULL)) {
            g_print("Повідомлення: %s\n", message->valuestring);
        }
    }


    
    //----
    g_string_free(response, TRUE);

cleanup:
    if (json) cJSON_Delete(json);
    if (json_data) g_free(json_data);
    if (request) g_free(request);
    if (json_response) cJSON_Delete(json_response);
    //if (uri) g_object_unref(uri);
    if (client) g_object_unref(client);
    if (connection) g_object_unref(connection);
    if (error) g_clear_error(&error);
}
