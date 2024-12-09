#include "chats.h"

int get_last_message_id_in_chat(sqlite3 *db, int chat_id) {
    const char *query =
        "SELECT last_message_id FROM chats WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare statement: %s", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    int last_message_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            last_message_id = sqlite3_column_int(stmt, 0);
        }
    } else {
        logging(WARN, "Chat ID %d not found or has no last_message_id", chat_id);
    }

    sqlite3_finalize(stmt);
    return last_message_id;
}
