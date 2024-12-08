#include "messages.h"

int get_chat_id_by_message_id(sqlite3 *db, int message_id) {
    const char *query = "SELECT chat_id FROM messages WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, message_id);

    int chat_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        chat_id = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return chat_id;
}
