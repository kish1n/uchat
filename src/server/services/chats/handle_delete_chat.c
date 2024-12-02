#include "chats.h"
#include "../../db/core/chats/chats.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>

int handle_delete_chat(HttpContext *context) {
    // Validate context
    if (!context) {
        logging(ERROR, "Invalid context passed to handle_delete_chat");
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
    struct json_object *chat_id_obj;
    int chat_id = -1;

    if (json_object_object_get_ex(parsed_json, "id", &chat_id_obj)) {
        chat_id = json_object_get_int(chat_id_obj);
    }

    // Validate required fields
    if (chat_id <= 0) {
        const char *error_msg = "Missing or invalid 'id' field";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Check if the chat exists
    if (!chat_exists(context->db_conn, chat_id)) {
        const char *error_msg = "Chat does not exist";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Delete the chat from the database
    int result = delete_chat(context->db_conn, chat_id);

    json_object_put(parsed_json);

    if (result == 0) {
        const char *success_msg = "{\"status\":\"success\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);

        logging(INFO, "Chat with id %d deleted successfully", chat_id);
        return ret;
    } else {
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Failed to delete chat\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);

        logging(ERROR, "Failed to delete chat with id %d", chat_id);
        return ret;
    }
}
