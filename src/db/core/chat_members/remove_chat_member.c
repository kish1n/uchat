#include "chat_members.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int remove_chat_member(PGconn *conn, int chat_id, int user_id) {
    if (!conn) {
        fprintf(stderr, "Invalid connection for remove_chat_member\n");
        return -1;
    }

    const char *query = "DELETE FROM chat_members WHERE chat_id = $1 AND user_id = $2;";
    const char *paramValues[2] = {NULL, NULL};

    char chat_id_str[12];
    char user_id_str[12];

    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);

    paramValues[0] = chat_id_str;
    paramValues[1] = user_id_str;

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error removing member from chat");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
