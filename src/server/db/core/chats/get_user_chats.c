#include "chats.h"
#include <sqlite3.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_user_chats(sqlite3 *db, const char *user_id) {
    const char *query =
        "SELECT c.id AS chat_id, c.name AS chat_name, c.last_message_id "
        "FROM chats c "
        "JOIN chat_members cm ON c.id = cm.chat_id "
        "WHERE cm.user_id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);

    struct json_object *response_array = json_object_new_array();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        struct json_object *chat_obj = json_object_new_object();
        json_object_object_add(chat_obj, "id", json_object_new_int(sqlite3_column_int(stmt, 0))); // chat_id
        const char *chat_name = (const char *)sqlite3_column_text(stmt, 1);
        json_object_object_add(chat_obj, "name", json_object_new_string(chat_name ? chat_name : "Unnamed Chat"));

        // Новое поле
        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
            json_object_object_add(chat_obj, "last_message_id", json_object_new_int(sqlite3_column_int(stmt, 2)));
        } else {
            json_object_object_add(chat_obj, "last_message_id", json_object_new_null());
        }

        json_object_array_add(response_array, chat_obj);
    }

    char *response = strdup(json_object_to_json_string(response_array));

    json_object_put(response_array);
    sqlite3_finalize(stmt);

    return response;
}
