#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include "messages.h"

int get_last_message_id(sqlite3 *db, int chat_id) {
    const char *query = "SELECT last_message_id FROM chats WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare statement: %s\nSQL: %s", sqlite3_errmsg(db), query);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    int last_message_id = -1; // Значение по умолчанию, если ID не найден
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        last_message_id = sqlite3_column_int(stmt, 0);
    } else {
        logging(WARN, "No last_message_id found for chat_id=%d", chat_id);
    }

    sqlite3_finalize(stmt);
    return last_message_id;
}
