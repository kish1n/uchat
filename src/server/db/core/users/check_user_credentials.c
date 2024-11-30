#include <string.h>
#include <libpq-fe.h>
#include "../../../pkg/crypto/crypto.h" // Для проверки пароля
#include "users.h"

int check_user_credentials(PGconn *conn, const char *username, const char *password) {
    const char *query = "SELECT passhash FROM users WHERE username = $1;";
    const char *paramValues[1] = { username };

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Database error: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0; // Ошибка в базе данных
    }

    if (PQntuples(res) == 0) {
        PQclear(res);
        return 0; // Пользователь не найден
    }

    const char *stored_passhash = PQgetvalue(res, 0, 0);

    // Проверка пароля
    int is_valid = verify_password(password, stored_passhash);

    PQclear(res);
    return is_valid; // 1 если пароль совпал, 0 если нет
}
