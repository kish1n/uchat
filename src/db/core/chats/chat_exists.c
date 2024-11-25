#include "chats.h"
#include <stdio.h>
#include <stdlib.h>
#include "../../../pkg/config/config.h"

int chat_exists(PGconn *conn, int chat_id) {
    if (!conn) {
        logging(ERROR, "Invalid database connection");
        return 0;
    }

    const char *query = "SELECT 1 FROM chats WHERE id = $1 LIMIT 1;";
    const char *paramValues[1] = {NULL};

    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        logging(ERROR, "Failed to execute chat_exists query: %s", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    int exists = PQntuples(res) > 0;
    PQclear(res);
    return exists;
}
