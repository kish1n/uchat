#include "service.h"
#include <stdlib.h>
#include <json-c/json.h>

enum MHD_Result handle_request(void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **con_cls) {
    (void)cls; (void)url; (void)version;

    if (*con_cls == NULL) {
        RequestData *request_data = calloc(1, sizeof(RequestData));
        if (!request_data) return MHD_NO;
        *con_cls = request_data;
        return MHD_YES;
    }

    RequestData *request_data = (RequestData *)*con_cls;

    if (strcmp(method, "POST") != 0) {
        const char *error_msg = "Only POST method is supported";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, response);
        MHD_destroy_response(response);
        return ret;
    }

    if (*upload_data_size > 0) {
        request_data->data = realloc(request_data->data, request_data->size + *upload_data_size + 1);
        if (!request_data->data) return MHD_NO;
        memcpy(request_data->data + request_data->size, upload_data, *upload_data_size);
        request_data->size += *upload_data_size;
        request_data->data[request_data->size] = '\0';
        *upload_data_size = 0;
        return MHD_YES;
    }

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

    const char *response_data = json_object_to_json_string_ext(parsed_json, JSON_C_TO_STRING_PLAIN);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_data), (void *)response_data, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    json_object_put(parsed_json);
    free_request_data(request_data);
    *con_cls = NULL;

    return ret;
}