#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "chat_members.h"

int add_chat_member(sqlite3 *db, int chat_id, const char *user_id, int is_admin) {
    const char *sql = "INSERT INTO chat_members (chat_id, user_id, is_admin, joined_at) VALUES (?, ?, ?, datetime('now'))";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }

    sqlite3_bind_int(stmt, 1, chat_id);
    sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, is_admin);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}
