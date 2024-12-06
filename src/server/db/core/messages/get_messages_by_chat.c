#include "messages.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_messages_by_chat(sqlite3 *db, int chat_id, Message **messages, int *message_count) {
    const char *query = "SELECT id, chat_id, sender_id, content, sent_at FROM messages WHERE chat_id = ? ORDER BY sent_at";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    int count = 0;
    Message *temp_messages = malloc(sizeof(Message) * 100); // Предполагаем максимум 100 сообщений

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        temp_messages[count].id = sqlite3_column_int(stmt, 0);
        temp_messages[count].chat_id = sqlite3_column_int(stmt, 1);
        strncpy(temp_messages[count].sender_id, (const char *)sqlite3_column_text(stmt, 2), sizeof(temp_messages[count].sender_id) - 1);
        strncpy(temp_messages[count].content, (const char *)sqlite3_column_text(stmt, 3), sizeof(temp_messages[count].content) - 1);
        strncpy(temp_messages[count].sent_at, (const char *)sqlite3_column_text(stmt, 4), sizeof(temp_messages[count].sent_at) - 1);
        count++;
    }

    sqlite3_finalize(stmt);

    *messages = temp_messages;
    *message_count = count;

    return 0;
}
