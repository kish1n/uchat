#include "chats.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_chat(sqlite3 *db, const char *chat_name, int is_group) {
    if (!db) {
        logging(ERROR, "Database connection is NULL");
        return -1;
    }

    if (!chat_name || strlen(chat_name) == 0) {
        logging(ERROR, "Chat name is NULL or empty");
        return -1;
    }

    const char *insert_query =
        "INSERT INTO chats (name, is_group, created_at) VALUES (?, ?, datetime('now'));";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, insert_query, -1, &stmt, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare INSERT statement: %s", sqlite3_errmsg(db));
        return -1;
    }

    if (sqlite3_bind_text(stmt, 1, chat_name, -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_int(stmt, 2, is_group) != SQLITE_OK) {
        logging(ERROR, "Failed to bind parameters: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
        }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        logging(ERROR, "Failed to execute INSERT statement: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    // Выполняем запрос для получения последнего ID
    const char *select_query = "SELECT last_insert_rowid();";
    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare SELECT statement: %s", sqlite3_errmsg(db));
        return -1;
    }

    int chat_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        chat_id = sqlite3_column_int(stmt, 0);
    } else {
        logging(ERROR, "Failed to retrieve last insert row ID: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    if (chat_id != -1) {
        logging(INFO, "Chat created successfully with ID: %d", chat_id);
    } else {
        logging(ERROR, "Chat creation failed for unknown reasons.");
    }

    return chat_id;
}
