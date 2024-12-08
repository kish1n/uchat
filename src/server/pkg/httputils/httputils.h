#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include <json-c/json.h>
#include "../../pkg/config/config.h"

int starts_with(const char *url, const char *prefix);
int process_request_data(HttpContext *context, struct json_object **parsed_json);

#endif