#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <json-c/json.h>

int get_json_value(const char *json_str, const char *key, char *value, size_t value_size);

#endif // JSON_PARSER_H
