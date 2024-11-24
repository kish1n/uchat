#include "chat_members.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int is_user_in_chat(PGconn *conn, int chat_id, int user_id) {
    if (!conn) {
        fprintf(stderr, "Invalid connection for is_user_in_chat\n");
        return -1;
    }

    const char *query = "SELECT 1 FROM chat_members WHERE chat_id = $1 AND user_id = $2;";
    const char *paramValues[2] = {NULL, NULL};

    char chat_id_str[12];
    char user_id_str[12];

    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);

    paramValues[0] = chat_id_str;
    paramValues[1] = user_id_str;

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        log_db_error(conn, "Error checking user in chat");
        PQclear(res);
        return -1;
    }

    int exists = PQntuples(res) > 0;
    PQclear(res);
    return exists;
}
