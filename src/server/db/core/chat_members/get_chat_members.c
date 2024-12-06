#include "chat_members.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_chat_members(PGconn *conn, int chat_id, ChatMember **members, int *member_count) {
    if (!conn || !members || !member_count) {
        fprintf(stderr, "Invalid parameters for get_chat_members\n");
        return -1;
    }

    const char *query =
        "SELECT user_id, is_admin, joined_at "
        "FROM chat_members WHERE chat_id = $1";

    const char *paramValues[1];
    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error fetching chat members: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    int rows = PQntuples(res);
    *members = malloc(rows * sizeof(ChatMember));
    *member_count = rows;

    for (int i = 0; i < rows; i++) {
        // user_id is now a string
        strncpy((*members)[i].user_id, PQgetvalue(res, i, 0), sizeof((*members)[i].user_id) - 1);
        (*members)[i].user_id[sizeof((*members)[i].user_id) - 1] = '\0';

        (*members)[i].is_admin = atoi(PQgetvalue(res, i, 1));

        strncpy((*members)[i].joined_at, PQgetvalue(res, i, 2), sizeof((*members)[i].joined_at) - 1);
        (*members)[i].joined_at[sizeof((*members)[i].joined_at) - 1] = '\0';
    }

    PQclear(res);
    return 0;
}
