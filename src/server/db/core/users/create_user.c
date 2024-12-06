#include "users.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_user(sqlite3 *db, const char *username, const char *passhash) {
    const char *query = "INSERT INTO users (id, username, passhash, created_at) "
                        "VALUES (lower(hex(randomblob(16))), ?, ?, datetime('now'))";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, passhash, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}
