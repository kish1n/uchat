#include "chats.h"

bool is_last_message_in_chat(sqlite3 *db, int message_id, int chat_id) {
    const char *query =
        "SELECT CASE WHEN last_message_id = ? THEN 1 ELSE 0 END "
        "FROM chats "
        "WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, message_id);
    sqlite3_bind_int(stmt, 2, chat_id);

    bool is_last = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        is_last = sqlite3_column_int(stmt, 0) == 1;
    }

    sqlite3_finalize(stmt);
    return is_last;
}
