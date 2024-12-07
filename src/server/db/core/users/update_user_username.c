#include "users.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int update_user_username(sqlite3 *db, const char *uuid, const char *new_username) {
    const char *query = "UPDATE users SET username = ? WHERE username = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, new_username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, uuid, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}