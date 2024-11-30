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

    const char *query = "SELECT chat_id, user_id, is_admin, joined_at FROM chat_members WHERE chat_id = $1;";
    const char *paramValues[1] = {NULL};

    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        log_db_error(conn, "Error fetching chat members");
        PQclear(res);
        return -1;
    }

    int rows = PQntuples(res);
    *members = malloc(rows * sizeof(ChatMember));
    *member_count = rows;

    for (int i = 0; i < rows; i++) {
        (*members)[i].chat_id = atoi(PQgetvalue(res, i, 0));
        (*members)[i].user_id = atoi(PQgetvalue(res, i, 1));
        (*members)[i].is_admin = strcmp(PQgetvalue(res, i, 2), "t") == 0 ? 1 : 0;
        strncpy((*members)[i].joined_at, PQgetvalue(res, i, 3), sizeof((*members)[i].joined_at) - 1);
        (*members)[i].joined_at[sizeof((*members)[i].joined_at) - 1] = '\0';
    }

    PQclear(res);
    return 0;
}
