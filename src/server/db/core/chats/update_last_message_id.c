#include "chats.h"

int update_last_message_id(sqlite3 *db, int chat_id) {
    const char *query =
        "SELECT id FROM messages "
        "WHERE chat_id = ? "
        "ORDER BY sent_at DESC LIMIT 1 OFFSET 1";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    int new_last_message_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        new_last_message_id = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    const char *update_query =
        "UPDATE chats SET last_message_id = ? WHERE id = ?";
    if (sqlite3_prepare_v2(db, update_query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare update statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    if (new_last_message_id == -1) {
        sqlite3_bind_null(stmt, 1);
    } else {
        sqlite3_bind_int(stmt, 1, new_last_message_id);
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
