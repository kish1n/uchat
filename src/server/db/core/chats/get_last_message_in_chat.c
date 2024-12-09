#include <sqlite3.h>
#include <stdio.h>
#include "chats.h"

int get_last_message_in_chat(sqlite3 *db, int chat_id) {
    const char *query =
        "SELECT id FROM messages "
        "WHERE chat_id = ? "
        "ORDER BY sent_at DESC LIMIT 1";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    int last_message_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        last_message_id = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return last_message_id;
}
