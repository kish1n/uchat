#include "users.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../../pkg/config/config.h"

User* get_user_by_username(sqlite3 *db, const char *username) {
    if (!db || !username) {
        fprintf(stderr, "Invalid arguments passed to get_user_by_username\n");
        return NULL;
    }

    const char *query = "SELECT id, username, passhash, created_at FROM users WHERE username = ?";
    sqlite3_stmt *stmt;

    // Подготовка запроса
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    // Привязка параметра
    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return NULL;
    }

    User *user = NULL;

    // Выполнение запроса
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        user = malloc(sizeof(User));
        if (!user) {
            fprintf(stderr, "Memory allocation failed for User\n");
            sqlite3_finalize(stmt);
            return NULL;
        }

        const char *id_text = (const char *)sqlite3_column_text(stmt, 0);
        const char *username_text = (const char *)sqlite3_column_text(stmt, 1);
        const char *passhash_text = (const char *)sqlite3_column_text(stmt, 2);
        const char *created_at_text = (const char *)sqlite3_column_text(stmt, 3);

        // Копирование данных
        user->id = id_text ? strdup(id_text) : NULL;
        user->username = username_text ? strdup(username_text) : NULL;
        user->passhash = passhash_text ? strdup(passhash_text) : NULL;
        user->created_at = created_at_text ? strdup(created_at_text) : NULL;

        // Проверка успешности копирования
        if (!user->id || !user->username || !user->passhash || !user->created_at) {
            fprintf(stderr, "Memory allocation failed for User fields\n");
            free(user->id);
            free(user->username);
            free(user->passhash);
            free(user->created_at);
            free(user);
            user = NULL;
        }
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error executing statement: %s\n", sqlite3_errmsg(db));
    }

    // Завершение запроса
    sqlite3_finalize(stmt);
    return user;
}
