#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "messages.h"
#include "../../../pkg/config/config.h"

int get_new_messages_by_chat(sqlite3 *db, int chat_id, int last_message_id, Message **messages, int *message_count) {
    if (!db || !messages || !message_count) {
        fprintf(stderr, "Invalid parameters to get_new_messages_by_chat\n");
        return -1;
    }

    const char *query =
        "SELECT id, chat_id, sender_id, content, sent_at "
        "FROM messages "
        "WHERE chat_id = ? AND id > ? "
        "ORDER BY sent_at ASC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);          // Привязка chat_id
    sqlite3_bind_int(stmt, 2, last_message_id);  // Привязка last_message_id

    // Массив для хранения сообщений
    int capacity = 10; // Начальный размер массива
    int count = 0;
    *messages = malloc(sizeof(Message) * capacity);
    if (!*messages) {
        fprintf(stderr, "Memory allocation failed\n");
        sqlite3_finalize(stmt);
        return -1;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (count >= capacity) {
            // Увеличиваем размер массива
            capacity *= 2;
            *messages = realloc(*messages, sizeof(Message) * capacity);
            if (!*messages) {
                fprintf(stderr, "Memory reallocation failed\n");
                sqlite3_finalize(stmt);
                return -1;
            }
        }

        Message *msg = &(*messages)[count];
        msg->id = sqlite3_column_int(stmt, 0);
        msg->chat_id = sqlite3_column_int(stmt, 1);
        strncpy(msg->sender_id, (const char *)sqlite3_column_text(stmt, 2), sizeof(msg->sender_id) - 1);
        msg->sender_id[sizeof(msg->sender_id) - 1] = '\0';
        strncpy(msg->content, (const char *)sqlite3_column_text(stmt, 3), sizeof(msg->content) - 1);
        msg->content[sizeof(msg->content) - 1] = '\0';
        strncpy(msg->sent_at, (const char *)sqlite3_column_text(stmt, 4), sizeof(msg->sent_at) - 1);
        msg->sent_at[sizeof(msg->sent_at) - 1] = '\0';

        count++;
    }

    sqlite3_finalize(stmt);

    // Устанавливаем итоговый размер массива
    *message_count = count;
    return 0; // Успешное выполнение
}
