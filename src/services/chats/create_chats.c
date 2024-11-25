#include "chats.h"
#include "../../db/core/chats/chats.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>

int handle_create_chat(HttpContext *context) {
    // Validate context
    if (!context) {
        logging(ERROR, "Invalid context passed to handle_create_chat");
        return MHD_NO;
    }

    // Allocate memory for incoming data if not already allocated
    if (*context->con_cls == NULL) {
        char *buffer = calloc(1, sizeof(char));
        *context->con_cls = buffer;
        return MHD_YES;
    }

    char *data = (char *)*context->con_cls;

    // Accumulate incoming data
    if (*context->upload_data_size > 0) {
        data = realloc(data, strlen(data) + *context->upload_data_size + 1);
        strncat(data, context->upload_data, *context->upload_data_size);
        *context->upload_data_size = 0;
        *context->con_cls = data;
        return MHD_YES;
    }

    // Parse the incoming JSON data
    struct json_object *parsed_json = json_tokener_parse(data);
    free(data);
    *context->con_cls = NULL;

    if (!parsed_json) {
        const char *error_msg = "Invalid JSON";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Extract required fields from JSON
    struct json_object *name_obj, *is_group_obj;
    const char *name = NULL;
    int is_group = 0;

    if (json_object_object_get_ex(parsed_json, "name", &name_obj)) {
        name = json_object_get_string(name_obj);
    }

    if (json_object_object_get_ex(parsed_json, "is_group", &is_group_obj)) {
        is_group = json_object_get_int(is_group_obj);
    }

    // Validate required fields
    if (!name || strlen(name) == 0) {
        const char *error_msg = "Missing or invalid 'name' field";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Save chat to database
    int result = create_chat(context->db_conn, name, is_group);

    json_object_put(parsed_json);

    if (result == 0) {
        const char *success_msg = "{\"status\":\"success\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);

        logging(INFO, "Chat '%s' created successfully", name);
        return ret;
    } else {
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Failed to create chat\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);

        logging(ERROR, "Failed to create chat '%s'", name);
        return ret;
    }
}
