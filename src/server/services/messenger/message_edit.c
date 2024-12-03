#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../service.h"
#include "../../pkg/config/config.h"
#include "../../pkg/httputils/httputils.h"
#include "../../pkg/http_response/response.h"
#include "../../db/core/messages/messages.h"
#include "../../db/core/chats/chats.h"

int handle_edit_message(HttpContext *context) {
    struct json_object *parsed_json = NULL;

    if (process_request_data(context, &parsed_json) != MHD_YES) {
        const char *error_msg = "Invalid JSON";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    struct json_object *message_id_obj, *new_content_obj;
    int message_id = -1;
    const char *new_content = NULL;

    if (json_object_object_get_ex(parsed_json, "message_id", &message_id_obj)) {
        message_id = json_object_get_int(message_id_obj);
    }

    if (json_object_object_get_ex(parsed_json, "new_content", &new_content_obj)) {
        new_content = json_object_get_string(new_content_obj);
    }

    if (message_id <= 0 || !new_content || strlen(new_content) == 0) {
        logging(ERROR, "Missing or invalid fields in edit message request");
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Missing or invalid fields\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    logging(DEBUG, "Editing message ID %d with new content: %s", message_id, new_content);

    // Check if the message exists
    Message message;
    if (get_message_by_id(context->db_conn, message_id, &message) != 0) {
        logging(ERROR, "Message not found: ID %d", message_id);
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Message not found\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Update the message
    if (edit_message(context->db_conn, message_id, new_content) != 0) {
        logging(ERROR, "Failed to update message content: ID %d", message_id);
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Failed to update message\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Success response
    const char *success_msg = "{\"status\":\"success\",\"message\":\"Message updated successfully\"}";
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    logging(INFO, "Message %d updated successfully", message_id);

    json_object_put(parsed_json);
    return ret;
}
