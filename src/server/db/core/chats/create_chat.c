#include "chats.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int create_chat(sqlite3 *db, const char *chat_name, int is_group) {
    const char *query =
        "INSERT INTO chats (name, is_group) VALUES (?, ?)";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }

    sqlite3_bind_text(stmt, 1, chat_name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, is_group);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_ERROR;
    }

    sqlite3_finalize(stmt);
    return SQLITE_OK;
}
