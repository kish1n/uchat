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
    Config cfg;
    load_config("config.yaml", &cfg);

    if (!context) {
        logging(ERROR, "Invalid context passed to handle_update_chat_name");
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

    struct json_object *parsed_json = json_tokener_parse(data);
    free(data);
    *context->con_cls = NULL;

    if (!parsed_json) {
        return prepare_simple_response("Invalid JSON", STATUS_BAD_REQUEST, NULL, context);
    }


    struct json_object *message_id_obj;
    int message_id = -1;

    if (json_object_object_get_ex(parsed_json, "id", &message_id_obj)) {
        message_id = json_object_get_int(message_id_obj);
    }

    // Validate required fields
    if (message_id <= 0) {
        return prepare_simple_response("Missing or invalid 'id' in request", STATUS_BAD_REQUEST, parsed_json, context);
    }

    // Check if the message exists
    if (!message_exists(context->db_conn, message_id)) {
        return prepare_simple_response("Message does not exist", STATUS_NOT_FOUND, parsed_json, context);
    }

    // Delete the message from the database
    int result = delete_message(context->db_conn, message_id);

    json_object_put(parsed_json);

    if (result != 0) {
        logging(ERROR, "Failed to delete message %d", message_id);
        return prepare_simple_response("Failed to delete message", STATUS_INTERNAL_SERVER_ERROR, NULL, context);
    }

    logging(INFO, "Message %d deleted successfully", message_id);

    return prepare_simple_response("Successfully deleted message", STATUS_OK, NULL, context);
}
