#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../service.h"
#include "../../pkg/httputils/httputils.h"
#include "../../pkg/http_response/response.h"
#include "../../db/core/messages/messages.h"
#include "../../db/core/chats/chats.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../db/core/users/users.h"


int handle_delete_message(HttpContext *context) {
    struct json_object *parsed_json = NULL;

    if (process_request_data(context, &parsed_json) != MHD_YES) {
        const char *error_msg = "Invalid JSON";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    struct json_object *message_id_obj;
    int message_id = -1;

    if (json_object_object_get_ex(parsed_json, "id", &message_id_obj)) {
        message_id = json_object_get_int(message_id_obj);
    }

    // Validate required fields
    if (message_id <= 0) {
        const char *error_msg = "Missing or invalid 'id' in request";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Check if the message exists
    if (!message_exists(context->db_conn, message_id)) {
        const char *error_msg = "Message does not exist";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        json_object_put(parsed_json);
        return ret;
    }

    // Delete the message from the database
    int result = delete_message(context->db_conn, message_id);

    json_object_put(parsed_json);

    if (result == 0) {
        const char *success_msg = "{\"status\":\"success\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);

        logging(INFO, "Message %d deleted successfully", message_id);
        return ret;
    } else {
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Failed to delete message\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);

        logging(ERROR, "Failed to delete message %d", message_id);
        return ret;
    }
}
