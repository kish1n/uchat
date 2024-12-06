#include "chats.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_chat_name(sqlite3 *db, int chat_id, char *chat_name, size_t chat_name_size) {
    const char *query = "SELECT name FROM chats WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        strncpy(chat_name, (const char *)sqlite3_column_text(stmt, 0), chat_name_size - 1);
        chat_name[chat_name_size - 1] = '\0';
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}
