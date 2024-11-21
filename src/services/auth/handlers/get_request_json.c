#include "handlers.h"
#include "../../../db/core/users/users.h"
#include <string.h>

// Структура для передачи данных между итератором и функцией
typedef struct {
    struct json_object *json_obj; // JSON-объект для хранения данных
} JsonRequestContext;

// Итератор для обработки ключ-значений
static enum MHD_Result iterator(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
    JsonRequestContext *context = (JsonRequestContext *)cls;

    // Если есть ключ и значение, добавляем их в JSON-объект
    if (key && value) {
        json_object_object_add(context->json_obj, key, json_object_new_string(value));
    }
    return MHD_YES;
}

// Функция для извлечения данных из тела запроса и создания JSON-объекта
struct json_object *get_request_json(struct MHD_Connection *connection) {
    JsonRequestContext context;
    context.json_obj = json_object_new_object(); // Создаём пустой JSON-объект

    if (!context.json_obj) {
        fprintf(stderr, "Failed to allocate memory for JSON object\n");
        return NULL;
    }

    // Вызываем MHD_get_connection_values с нашим итератором
    int result = MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, iterator, &context);

    // Если итератор не вызвался или данные не были добавлены
    if (result == MHD_NO || json_object_object_length(context.json_obj) == 0) {
        fprintf(stderr, "No data in request or failed to parse arguments\n");
        json_object_put(context.json_obj); // Освобождаем JSON объект
        return NULL;
    }

    return context.json_obj; // Возвращаем созданный JSON-объект
}