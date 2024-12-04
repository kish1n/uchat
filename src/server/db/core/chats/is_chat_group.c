#include "chats.h"
#include <stdlib.h>
#include <stdio.h>
#include <libpq-fe.h>

int is_chat_group(PGconn *conn, int chat_id) {
    if (!conn || chat_id <= 0) {
        fprintf(stderr, "Invalid parameters for is_chat_group\n");
        return 0;
    }

    const char *query =
        "SELECT 1 FROM chats WHERE id = $1 AND is_group = true";

    const char *paramValues[1];
    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error checking if chat is a group: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    int is_group = (PQntuples(res) > 0); // Если есть результат, то это группа

    PQclear(res);
    return is_group;
}
