#include "chats.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int chat_exists(sqlite3 *db, int chat_id) {
    const char *query =
        "SELECT COUNT(*) FROM chats WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0; // Возвращаем 0, так как это означает "не существует"
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    int exists = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return exists;
}
