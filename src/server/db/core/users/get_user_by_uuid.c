#include "users.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../../pkg/config/config.h"

User* get_user_by_uuid(sqlite3 *db, const char *uuid) {
    const char *query = "SELECT id, username, passhash, created_at FROM users WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, uuid, -1, SQLITE_STATIC);

    User *user = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = malloc(sizeof(User));
        user->id = strdup((const char *)sqlite3_column_text(stmt, 0));
        user->username = strdup((const char *)sqlite3_column_text(stmt, 1));
        user->passhash = strdup((const char *)sqlite3_column_text(stmt, 2));
        user->created_at = strdup((const char *)sqlite3_column_text(stmt, 3));
    }

    sqlite3_finalize(stmt);
    return user;
}
