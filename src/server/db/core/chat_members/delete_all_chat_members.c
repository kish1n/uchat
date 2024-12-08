#include <stdio.h>
#include <sqlite3.h>
#include "chat_members.h"

int delete_all_chat_members(sqlite3 *db, int chat_id) {
    const char *sql = "DELETE FROM chat_members WHERE chat_id = ?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}