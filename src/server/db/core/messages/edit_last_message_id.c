#include <sqlite3.h>
#include <stdio.h>
#include "../chats/chats.h"
#include "../../../pkg/config/config.h"

int edit_last_message_id(sqlite3 *db, int chat_id, int message_id) {
    const char *update_query = "UPDATE chats SET last_message_id = ? WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, update_query, -1, &stmt, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare statement: %s\nSQL: %s", sqlite3_errmsg(db), update_query);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, message_id);
    sqlite3_bind_int(stmt, 2, chat_id);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        logging(ERROR, "Failed to execute statement: %s\nSQL: %s", sqlite3_errmsg(db), update_query);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    logging(INFO, "Updated last_message_id for chat_id=%d to message_id=%d", chat_id, message_id);
    return 0;
}
