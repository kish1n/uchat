#include "chat_members.h"
#include <stdlib.h>
#include <stdio.h>

int is_user_admin(PGconn *conn, int chat_id, const char *user_id) {
    if (!conn || !user_id) {
        fprintf(stderr, "Invalid parameters for is_user_admin\n");
        return 0; // Не является администратором
    }

    const char *query =
        "SELECT 1 "
        "FROM chat_members "
        "WHERE chat_id = $1 AND user_id::uuid = $2 AND is_admin = TRUE";

    const char *paramValues[2];
    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);

    paramValues[0] = chat_id_str;
    paramValues[1] = user_id;

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error checking if user is admin: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0; // Не является администратором
    }

    int is_admin = PQntuples(res) > 0 ? 1 : 0; // Проверка наличия строки

    PQclear(res);
    return is_admin;
}
