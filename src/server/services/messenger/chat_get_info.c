#include "messenger.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../db/core/chats/chats.h"
#include "../../db/core/chat_members/chat_members.h"
#include "../../pkg/http_response/response.h"
#include "../../pkg/httputils/httputils.h"
#include "../../db/core/users/users.h"

int handle_get_chat_info(HttpContext *context) {
    if (!context) {
        logging(ERROR, "Invalid context passed to handle_get_chat_info");
        return MHD_NO;
    }

    const char *url = context->url;
    int chat_id = atoi(url);

    if (chat_id <= 0) {
        return prepare_simple_response("Invalid or missing 'chat_id'", STATUS_BAD_REQUEST, NULL, context);
    }

    // Check if the chat exists
    if (!chat_exists(context->db_conn, chat_id)) {
        return prepare_simple_response("Chat does not exist", STATUS_NOT_FOUND, NULL, context);
    }

    // Get chat name
    char chat_name[255];
    if (get_chat_name(context->db_conn, chat_id, chat_name, sizeof(chat_name)) != 0) {
        logging(ERROR, "Failed to retrieve chat name for chat ID: %d", chat_id);
        return prepare_simple_response("Failed to retrieve chat name", STATUS_INTERNAL_SERVER_ERROR, NULL, context);;
    }

    // Get chat members
    ChatMember *members = NULL;
    int member_count = 0;

    if (get_chat_members(context->db_conn, chat_id, &members, &member_count) != 0) {
        logging(ERROR, "Failed to retrieve chat members for chat ID: %d", chat_id);
        return prepare_simple_response("Failed to retrieve chat members", STATUS_INTERNAL_SERVER_ERROR, NULL, context);
    }

    // Build JSON response
    struct json_object *response_json = json_object_new_object();
    struct json_object *members_array = json_object_new_array();

    for (int i = 0; i < member_count; ++i) {
        User *sender = get_user_by_uuid(context->db_conn, members[i].user_id);
        struct json_object *member_obj = json_object_new_object();
        json_object_object_add(member_obj, "username", json_object_new_string(sender->username));
        json_object_object_add(member_obj, "is_admin", json_object_new_boolean(members[i].is_admin));
        json_object_array_add(members_array, member_obj);
        free_user(sender);
    }

    json_object_object_add(response_json, "chat_name", json_object_new_string(chat_name));
    json_object_object_add(response_json, "members", members_array);

    logging(INFO, "Retrieved information for chat ID: %d", chat_id);

    free(members);

    return prepare_response( STATUS_OK, response_json, context);;
}
