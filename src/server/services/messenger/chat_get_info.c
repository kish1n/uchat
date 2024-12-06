#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../db/core/chats/chats.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../pkg/http_response/response.h"
#include "../../pkg/httputils/httputils.h"

int handle_get_chat_info(HttpContext *context) {
    if (!context) {
        logging(ERROR, "Invalid context passed to handle_get_chat_info");
        return MHD_NO;
    }

    const char *url = context->url;
    int chat_id = atoi(url);

    if (chat_id <= 0) {
        const char *error_msg = create_error_response("Invalid or missing 'chat_id'", STATUS_BAD_REQUEST);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Check if the chat exists
    if (!chat_exists(context->db_conn, chat_id)) {
        const char *error_msg = create_error_response("Chat does not exist", STATUS_NOT_FOUND);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Get chat name
    char chat_name[255];
    if (get_chat_name(context->db_conn, chat_id, chat_name, sizeof(chat_name)) != 0) {
        logging(ERROR, "Failed to retrieve chat name for chat ID: %d", chat_id);
        const char *error_msg = create_error_response("Failed to retrieve chat name", STATUS_INTERNAL_SERVER_ERROR);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Get chat members
    ChatMember *members = NULL;
    int member_count = 0;

    if (get_chat_members(context->db_conn, chat_id, &members, &member_count) != 0) {
        logging(ERROR, "Failed to retrieve chat members for chat ID: %d", chat_id);
        const char *error_msg = create_error_response("Failed to retrieve chat members", STATUS_INTERNAL_SERVER_ERROR);
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Build JSON response
    struct json_object *response_json = json_object_new_object();
    struct json_object *members_array = json_object_new_array();

    for (int i = 0; i < member_count; ++i) {
        struct json_object *member_obj = json_object_new_object();
        json_object_object_add(member_obj, "user_id", json_object_new_string(members[i].user_id));
        json_object_object_add(member_obj, "is_admin", json_object_new_boolean(members[i].is_admin));
        json_object_array_add(members_array, member_obj);
    }

    json_object_object_add(response_json, "chat_name", json_object_new_string(chat_name));
    json_object_object_add(response_json, "members", members_array);

    const char *response_str = json_object_to_json_string(response_json);
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_str), (void *)response_str, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    logging(INFO, "Retrieved information for chat ID: %d", chat_id);

    json_object_put(response_json);
    free(members);

    return ret;
}
