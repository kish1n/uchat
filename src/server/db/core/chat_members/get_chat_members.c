#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "chat_members.h"

int get_chat_members(sqlite3 *db, int chat_id, ChatMember **members, int *member_count) {
    const char *sql = "SELECT user_id, is_admin, joined_at FROM chat_members WHERE chat_id = ?";
    sqlite3_stmt *stmt;
    int count = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    ChatMember *result = malloc(sizeof(ChatMember) * 10); // Initial size, expand if needed
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        sqlite3_finalize(stmt);
        return SQLITE_NOMEM;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (count % 10 == 0 && count > 0) {
            result = realloc(result, sizeof(ChatMember) * (count + 10));
            if (!result) {
                fprintf(stderr, "Memory allocation failed\n");
                sqlite3_finalize(stmt);
                return SQLITE_NOMEM;
            }
        }

        strncpy(result[count].user_id, (const char *)sqlite3_column_text(stmt, 0), sizeof(result[count].user_id) - 1);
        result[count].is_admin = sqlite3_column_int(stmt, 1);
        strncpy(result[count].joined_at, (const char *)sqlite3_column_text(stmt, 2), sizeof(result[count].joined_at) - 1);
        result[count].chat_id = chat_id;
        count++;
    }

    sqlite3_finalize(stmt);

    *members = result;
    *member_count = count;

    return SQLITE_OK;
}
