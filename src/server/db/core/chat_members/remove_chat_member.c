#include "chat_members.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int remove_chat_member(PGconn *conn, int chat_id, const char *user_id) {
    if (!conn || !user_id) {
        fprintf(stderr, "Invalid parameters for remove_chat_member\n");
        return -1;
    }

    const char *query =
        "DELETE FROM chat_members "
        "WHERE chat_id = $1 AND user_id = $2";

    const char *paramValues[2];
    char chat_id_str[12];

    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;
    paramValues[1] = user_id;

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Error removing chat member: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
