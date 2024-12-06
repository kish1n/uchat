#include "chats.h"
#include <sqlite3.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_chat_messages(sqlite3 *db, int chat_id) {
    const char *query =
        "SELECT m.id AS message_id, u.username AS sender, m.content AS message, m.sent_at "
        "FROM messages m "
        "JOIN users u ON m.sender_id = u.id "
        "WHERE m.chat_id = ? "
        "ORDER BY m.sent_at ASC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    struct json_object *response_array = json_object_new_array();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        struct json_object *message_obj = json_object_new_object();

        json_object_object_add(
            message_obj, "id",
            json_object_new_string((const char *)sqlite3_column_text(stmt, 0))); // message_id
        json_object_object_add(
            message_obj, "sender",
            json_object_new_string((const char *)sqlite3_column_text(stmt, 1))); // sender
        json_object_object_add(
            message_obj, "message",
            json_object_new_string((const char *)sqlite3_column_text(stmt, 2))); // message
        json_object_object_add(
            message_obj, "sent_at",
            json_object_new_string((const char *)sqlite3_column_text(stmt, 3))); // sent_at

        json_object_array_add(response_array, message_obj);
    }

    char *response = strdup(json_object_to_json_string(response_array));

    json_object_put(response_array);
    sqlite3_finalize(stmt);

    return response;
}
