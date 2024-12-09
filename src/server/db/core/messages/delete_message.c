#include "messages.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int delete_message(sqlite3 *db, int message_id) {
    const char *query = "DELETE FROM messages WHERE id = ?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, message_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}
