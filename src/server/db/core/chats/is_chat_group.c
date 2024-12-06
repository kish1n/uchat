#include "chats.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int is_chat_group(sqlite3 *db, int chat_id) {
    const char *query =
        "SELECT is_group FROM chats WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0; // 0 означает, что чат не является группой
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    int is_group = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        is_group = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return is_group;
}
