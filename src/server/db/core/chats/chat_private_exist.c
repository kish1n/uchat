#include "chats.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int private_chat_exist(sqlite3 *db, const char *user1_id, const char *user2_id) {
    const char *query =
        "SELECT COUNT(*) "
        "FROM chats c "
        "JOIN chat_members cm1 ON c.id = cm1.chat_id "
        "JOIN chat_members cm2 ON c.id = cm2.chat_id "
        "WHERE cm1.user_id = ? AND cm2.user_id = ? AND c.is_group = 0";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0; // Возвращаем 0, если приватный чат не существует
    }

    sqlite3_bind_text(stmt, 1, user1_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user2_id, -1, SQLITE_STATIC);

    int exists = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return exists;
}
