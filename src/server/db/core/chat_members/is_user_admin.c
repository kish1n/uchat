#include <stdio.h>
#include <sqlite3.h>
#include "chat_members.h"

int is_user_admin(sqlite3 *db, int chat_id, const char *user_id) {
    const char *sql = "SELECT is_admin FROM chat_members WHERE chat_id = ? AND user_id = ?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);
    sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);

    int is_admin = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        is_admin = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return is_admin;
}
