#include "messages.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int create_message(sqlite3 *db, int chat_id, const char *sender_id, const char *content) {
    const char *query = "INSERT INTO messages (chat_id, sender_id, content, sent_at) "
                        "VALUES (?, ?, ?, datetime('now'))";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare statement: %s\nSQL: %s", sqlite3_errmsg(db), query);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);
    sqlite3_bind_text(stmt, 2, sender_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, content, -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        logging(ERROR, "Failed to execute statement: %s\nSQL: %s", sqlite3_errmsg(db), query);
        sqlite3_finalize(stmt);
        return -1;
    }

    int message_id = (int)sqlite3_last_insert_rowid(db); // Получаем ID созданного сообщения
    sqlite3_finalize(stmt);

    // Используем edit_last_message_id для обновления чата
    if (edit_last_message_id(db, chat_id, message_id) != 0) {
        logging(ERROR, "Failed to update last_message_id for chat_id=%d", chat_id);
        return -1;
    }

    logging(INFO, "Message created: chat_id=%d, sender_id=%s, content=%s, message_id=%d",
            chat_id, sender_id, content, message_id);
    return message_id;
}
