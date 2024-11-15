#include "json_parser.h"
#include <stdio.h>
#include <string.h>

int get_json_value(const char *json_str, const char *key, char *value, size_t value_size) {
    struct json_object *parsed_json = json_tokener_parse(json_str);
    if (!parsed_json) {
        fprintf(stderr, "Error: Failed to parse JSON\n");
        return -1;
    }

    // Извлекаем значение по ключу
    struct json_object *json_value;
    if (json_object_object_get_ex(parsed_json, key, &json_value)) {
        const char *str_value = json_object_get_string(json_value);
        if (str_value) {
            strncpy(value, str_value, value_size - 1);
            value[value_size - 1] = '\0';
            json_object_put(parsed_json);
            return 0;
        }
    }

    fprintf(stderr, "Error: Key '%s' not found in JSON\n", key);
    json_object_put(parsed_json);
    return -1;
}
