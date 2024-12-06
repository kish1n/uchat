#include "messages.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_message_by_id(sqlite3 *db, int message_id, Message *message) {
    const char *query = "SELECT id, chat_id, sender_id, content, sent_at FROM messages WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, message_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        message->id = sqlite3_column_int(stmt, 0);
        message->chat_id = sqlite3_column_int(stmt, 1);
        strncpy(message->sender_id, (const char *)sqlite3_column_text(stmt, 2), sizeof(message->sender_id) - 1);
        strncpy(message->content, (const char *)sqlite3_column_text(stmt, 3), sizeof(message->content) - 1);
        strncpy(message->sent_at, (const char *)sqlite3_column_text(stmt, 4), sizeof(message->sent_at) - 1);

        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}
