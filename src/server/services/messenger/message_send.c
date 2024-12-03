#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../service.h"
#include "../../pkg/httputils/httputils.h"
#include "../../pkg/http_response/response.h"
#include "../../db/core/messages/messages.h"
#include "../../db/core/chats/chats.h"

int handle_send_message(HttpContext *context) {
    struct json_object *parsed_json = NULL;

    if (process_request_data(context, &parsed_json) != MHD_YES) {
        const char *error_msg = "Invalid JSON";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    struct json_object *chat_id_obj, *sender_id_obj, *content_obj;
    int chat_id = -1;
    int sender_id = -1;
    const char *content = NULL;

    if (json_object_object_get_ex(parsed_json, "chat_id", &chat_id_obj) &&
        json_object_object_get_ex(parsed_json, "sender_id", &sender_id_obj) &&
        json_object_object_get_ex(parsed_json, "content", &content_obj)) {
        chat_id = json_object_get_int(chat_id_obj);
        sender_id = json_object_get_int(sender_id_obj);
        content = json_object_get_string(content_obj);
    }

    // Validate required fields
    if (chat_id <= 0 || sender_id <= 0 || !content || strlen(content) == 0) {
        const char *error_msg = "Missing or invalid fields in request";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *) error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Check if chat exists
    if (!chat_exists(context->db_conn, chat_id)) {
        const char *error_msg = "Chat does not exist";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *) error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Save message to database
    int result = create_message(context->db_conn, chat_id, sender_id, content);

    json_object_put(parsed_json);

    if (result == 0) {
        const char *success_msg = "{\"status\":\"success\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(success_msg), (void *) success_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);

        logging(INFO, "Message sent successfully by user %d in chat %d", sender_id, chat_id);
        return ret;
    } else {
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Failed to send message\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *) error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);

        logging(ERROR, "Failed to send message by user %d in chat %d", sender_id, chat_id);
        return ret;
    }
}
