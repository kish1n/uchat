#include "chats.h"

int update_last_message_id(sqlite3 *db, int chat_id, int message_id) {
    const char *query =
        "UPDATE chats SET last_message_id = ? WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare update statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    if (message_id == -1) {
        sqlite3_bind_null(stmt, 1);
    } else {
        sqlite3_bind_int(stmt, 1, message_id);
    }

    sqlite3_bind_int(stmt, 2, chat_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute update statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}
