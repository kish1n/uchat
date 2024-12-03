#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

int chat_exists(PGconn *conn, int chat_id) {
    if (!conn) {
        fprintf(stderr, "Invalid database connection in chat_exists\n");
        return 0;
    }

    const char *query = "SELECT 1 FROM chats WHERE id = $1;";
    const char *paramValues[1];
    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error checking chat existence: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    int exists = PQntuples(res) > 0; // Если хотя бы одна строка вернулась, чат существует
    PQclear(res);
    return exists;
}
