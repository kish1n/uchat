#include "chats.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_chat_by_id(sqlite3 *db, int chat_id, Chat *chat) {
    const char *query = "SELECT id, name, is_group, created_at FROM chats WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        chat->id = sqlite3_column_int(stmt, 0);
        strncpy(chat->name, (const char *)sqlite3_column_text(stmt, 1), sizeof(chat->name) - 1);
        chat->name[sizeof(chat->name) - 1] = '\0';
        chat->is_group = sqlite3_column_int(stmt, 2);
        strncpy(chat->created_at, (const char *)sqlite3_column_text(stmt, 3), sizeof(chat->created_at) - 1);
        chat->created_at[sizeof(chat->created_at) - 1] = '\0';

        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}
