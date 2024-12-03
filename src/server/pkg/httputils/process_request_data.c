#include "httputils.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>

int process_request_data(HttpContext *context, struct json_object **parsed_json) {
    if (!context) {
        logging(ERROR, "Context is NULL");
        return MHD_NO;
    }

    // Проверяем, есть ли данные
    if (!context->upload_data || *context->upload_data_size == 0) {
        logging(ERROR, "No data in upload_data or upload_data_size is zero");
        return MHD_NO;
    }

    // Печатаем сырые данные для отладки
    logging(DEBUG, "Raw data received: %s", context->upload_data);

    // Парсим JSON
    *parsed_json = json_tokener_parse(context->upload_data);
    if (!*parsed_json) {
        logging(ERROR, "Failed to parse JSON: %s", context->upload_data);
        return MHD_NO;
    }

    return MHD_YES;
}
