#include "response.h"

/**
 * Prepares an HTTP response with the given error message, status code, and JSON data.
 */

int prepare_simple_response(const char *error_msg, const STATUS_CODE status_code,  json_object *json_data, HttpContext *context) {
    const char *response_str = create_response(error_msg, status_code);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_str), (void *)response_str, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(context->connection, status_code, response);
    MHD_destroy_response(response);
    json_object_put(json_data);
    return ret;
}

int prepare_response(const STATUS_CODE status_code,  json_object *json_data, HttpContext *context) {
    const char *response_str = json_object_to_json_string(json_data);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_str), (void *)response_str, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(context->connection, status_code, response);
    MHD_destroy_response(response);
    json_object_put(json_data);
    return ret;
}