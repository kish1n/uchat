#include "client.h"
#include <string.h>


int chat_id_global;

#include <stdio.h>
#include <string.h>

char *extract_other_username(const char *input, const char *my_username) {
    // Перевірка вхідних параметрів
    if (!input || !my_username) {
        return NULL;
    }

    // Перевірити, чи містить рядок "private"
    if (strncmp(input, "private_", 8) != 0) {
        // Якщо "private" не знайдено, повернути копію вхідного рядка
        return strdup(input);
    }

    // Знайти перше і друге підкреслення
    const char *first_underscore = strchr(input, '_');
    if (!first_underscore) {
        return strdup(input); // Якщо щось пішло не так, повернути вхідний рядок
    }

    const char *second_underscore = strchr(first_underscore + 1, '_');
    if (!second_underscore) {
        return strdup(input); // Якщо друге підкреслення не знайдено, повернути вхідний рядок
    }

    // Отримати два можливих імені
    const char *user1 = first_underscore + 1; // Перше ім'я після "private_"
    const char *user2 = second_underscore + 1; // Друге ім'я після другого підкреслення

    // Визначити довжину імен
    size_t user1_length = second_underscore - user1;
    size_t user2_length = strlen(user2);

    // Порівняти з власним юзернеймом
    if (strncmp(user1, my_username, user1_length) == 0 && strlen(my_username) == user1_length) {
        return strndup(user2, user2_length); // Повернути друге ім'я
    } else if (strcmp(user2, my_username) == 0) {
        return strndup(user1, user1_length); // Повернути перше ім'я
    }

    // Якщо чужий нік не знайдено, повернути копію оригінального рядка
    return strdup(input);
}


void set_chat_id_global(int chat_id) {
    chat_id_global = chat_id;
}

int get_chat_id_global() {
    if (chat_id_global == -1) {
        fprintf(stderr, "Попередження: chat_id_global ще не встановлений.\n");
        return -1; // Повертаємо -1, якщо значення ще не встановлене
    }

    return chat_id_global;
}

// Функція для виділення тіла JSON з HTTP-відповіді
char *extract_json_from_response_2(const char *response_str) {
    if (!response_str) {
        fprintf(stderr, "Помилка: response_str є NULL.\n");
        return NULL;
    }

    // Знайти роздільник між заголовками та тілом (\r\n\r\n)
    const char *body = strstr(response_str, "\r\n\r\n");
    if (!body) {
        fprintf(stderr, "Помилка: роздільник \\r\\n\\r\\n не знайдено у відповіді.\n");
        return NULL;
    }

    // Перейти до початку JSON (пропустити \r\n\r\n)
    body += 4;

    // Створити копію JSON-тіла
    char *json_body = strdup(body);
    if (!json_body) {
        fprintf(stderr, "Помилка виділення пам'яті для JSON-тіла.\n");
        return NULL;
    }

    return json_body;
}

// Функція для надсилання запиту на створення приватного чату
void create_private_chat(const char *url, const char *token, const char *username) {
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
    cJSON_AddStringToObject(json, "with_user", username);
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
    size_t request_size = strlen(json_data) + strlen(token) + 300;
    request = g_malloc0(request_size);
    snprintf(request, request_size,
             "POST %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             g_uri_get_path(uri), g_uri_get_host(uri), token, strlen(json_data), json_data);

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
    
    
    // Аналіз відповіді сервера
    
    cJSON *json_response = cJSON_Parse(extract_json_from_response_2(response->str));
    if (cJSON_GetArraySize(json_response) == 1) {

        cJSON *id = cJSON_GetObjectItem(json_response, "chat_id");
        int id_int = id->valueint;
        set_chat_id_global(id_int);
        add_room_to_list(username, id_int);
    }
    else{
        g_print("\n");
    }
    
    g_string_free(response, TRUE);

cleanup:
    if (json) cJSON_Delete(json);
    if (json_data) g_free(json_data);
    if (request) g_free(request);
    if (client) g_object_unref(client);
    if (connection) g_object_unref(connection);
    if (error) g_clear_error(&error);
}


// Функція для створення групового чату
void create_group_chat(const char *url, const char *token, const char *group_name, const char *members) {
    GError *error = NULL;
    GSocketClient *client = g_socket_client_new();
    GSocketConnection *connection = NULL;
    GOutputStream *output_stream = NULL;
    GInputStream *input_stream = NULL;
    char *json_data = NULL;
    char *request = NULL;

    // Створюємо JSON-об'єкт
    cJSON *root = cJSON_CreateObject();
    cJSON *json_array = cJSON_CreateArray();

    // Розділяємо рядок користувачів по комі та додаємо до масиву
    char *members_copy = strdup(members); // Робимо копію, оскільки strtok змінює вхідний рядок
    char *substring = strtok(members_copy, ",");
    while (substring != NULL) {
        while (*substring == ' ') substring++; // Видаляємо зайві пробіли
        cJSON_AddItemToArray(json_array, cJSON_CreateString(substring));
        substring = strtok(NULL, ",");
    }
    free(members_copy);

    // Додаємо дані до JSON
    cJSON_AddItemToObject(root, "users", json_array);
    cJSON_AddStringToObject(root, "name", group_name);

    // Перетворюємо JSON на рядок
    json_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

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
    size_t request_size = strlen(json_data) + strlen(token) + 300;
    request = g_malloc0(request_size);
    snprintf(request, request_size,
             "POST %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             g_uri_get_path(uri), g_uri_get_host(uri), token, strlen(json_data), json_data);

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

    // Аналіз відповіді сервера
    
    cJSON *json_response = cJSON_Parse(extract_json_from_response_2(response->str));

    if (cJSON_GetArraySize(json_response) == 1) {

        cJSON *id = cJSON_GetObjectItem(json_response, "chat_id");
        int id_int = id->valueint;
        set_chat_id_global(id_int);
        add_room_to_list(group_name, id_int);
    }
    else{
        g_print("\n");
    }
    
    g_string_free(response, TRUE);

cleanup:
    if (json_data) g_free(json_data);
    if (request) g_free(request);
    if (client) g_object_unref(client);
    if (connection) g_object_unref(connection);
    if (error) g_clear_error(&error);
}

void send_message(const char *url, const char *token, const char *chat_id, const char *content) {
    GError *error = NULL;
    GSocketClient *client = g_socket_client_new();
    GSocketConnection *connection = NULL;
    GOutputStream *output_stream = NULL;
    GInputStream *input_stream = NULL;
    char *json_data = NULL;
    char *request = NULL;
    // Створення JSON-об'єкта
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "chat_id", chat_id);
    cJSON_AddStringToObject(root, "content", content);

    // Перетворення JSON на рядок
    json_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

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
    size_t request_size = strlen(json_data) + strlen(token) + 300;
    request = g_malloc0(request_size);
    snprintf(request, request_size,
             "POST %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             g_uri_get_path(uri), g_uri_get_host(uri), token, strlen(json_data), json_data);
    
    
    // Відправка запиту
    g_output_stream_write_all(output_stream, request, strlen(request), NULL, NULL, &error);
    if (error) {
        g_printerr("Помилка відправки даних: %s\n", error->message);
        goto cleanup;
    } else {
        g_print("Повідомлення відправлено успішно.\n");
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

    // Аналіз відповіді сервера
    cJSON *json_response = cJSON_Parse(extract_json_from_response_2(response->str));
    if (json_response) {
        cJSON *status = cJSON_GetObjectItemCaseSensitive(json_response, "status");
        if (cJSON_IsString(status) && (status->valuestring != NULL)) {
            if (strcmp(status->valuestring, "success") == 0) {
                g_print("Повідомлення успішно відправлено до чату %s.\n", chat_id);
            } else {
                g_print("Відправка повідомлення не вдалася: %s\n", status->valuestring);
            }
        }
        cJSON_Delete(json_response);
    }

    g_string_free(response, TRUE);

cleanup:
    if (json_data) g_free(json_data);
    if (request) g_free(request);
    if (client) g_object_unref(client);
    if (connection) g_object_unref(connection);
    if (error) g_clear_error(&error);
}

void fetch_and_load_chats(const char *url) {
    GError *error = NULL;
    GSocketClient *client = g_socket_client_new();
    GSocketConnection *connection = NULL;
    GOutputStream *output_stream = NULL;
    GInputStream *input_stream = NULL;
    char *request = NULL;

  

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
    size_t request_size = 256;
    request = g_malloc0(request_size);
    snprintf(request, request_size,
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             g_uri_get_path(uri), g_uri_get_host(uri), get_global_token());

    // Відправка запиту
    if (!g_output_stream_write_all(output_stream, request, strlen(request), NULL, NULL, &error)) {
        g_printerr("Помилка відправки запиту: %s\n", error->message);
        goto cleanup;
    } else {
        g_print("Запит на отримання чатів відправлено успішно.\n");
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
    
    // Парсинг JSON із відповіді
    const char *json_data = extract_json_from_response_2(response->str);
    
    cJSON *json_response = cJSON_Parse(json_data);
    
    if (!json_response || !cJSON_IsArray(json_response)) {
        g_printerr("Помилка парсингу JSON або відповідь не є масивом.\n");
        goto cleanup;
    }
    
    // Обробка кожного елементу масиву
    cJSON *chat = NULL;
    cJSON_ArrayForEach(chat, json_response) {
        cJSON *name = cJSON_GetObjectItem(chat, "name");
        cJSON *id = cJSON_GetObjectItem(chat, "id");
        if (name && cJSON_IsString(name)) {
            // Додаємо чат у список
            add_room_to_list(extract_other_username(name->valuestring, get_username_global()), id->valueint);
        }
    }

cleanup:
    g_print("\n");
}



void update_username_request(const char *url, const char *token, const char *new_username, const char *password) {
    GError *error = NULL;
    GSocketClient *client = g_socket_client_new();
    GSocketConnection *connection = NULL;
    GOutputStream *output_stream = NULL;
    GInputStream *input_stream = NULL;
    char *request = NULL;

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

    // Формування JSON для тіла запиту
    char json_data[256];
    snprintf(json_data, sizeof(json_data), "{\"new_username\": \"%s\", \"password\": \"%s\"}", new_username, password);

    // Формування HTTP-запиту
    size_t request_size = 512;
    request = g_malloc0(request_size);
    snprintf(request, request_size,
             "PATCH %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             g_uri_get_path(uri), g_uri_get_host(uri), token, strlen(json_data), json_data);

    // Відправка запиту
    if (!g_output_stream_write_all(output_stream, request, strlen(request), NULL, NULL, &error)) {
        g_printerr("Помилка відправки запиту: %s\n", error->message);
        goto cleanup;
    } else {
        g_print("Запит на зміну нікнейму відправлено успішно.\n");
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

cleanup:
    g_print("Without valgrint\n");
}
