#include <string.h>
#include <libpq-fe.h>
#include "../../../pkg/crypto/crypto.h" // Для проверки пароля
#include "users.h"

#include <string.h>
#include "../../../pkg/crypto/crypto.h" // Для проверки пароля
#include "users.h"

int check_user_credentials(sqlite3 *db, const char *username, const char *password) {
    const char *query = "SELECT passhash FROM users WHERE username = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    int result = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *stored_hash = (const char *)sqlite3_column_text(stmt, 0);
        result = verify_password(password, stored_hash); // Используем функцию проверки хэша
    }

    sqlite3_finalize(stmt);
    return result;
}
