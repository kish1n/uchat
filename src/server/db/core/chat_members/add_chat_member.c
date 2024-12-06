#include "chat_members.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int add_chat_member(PGconn *conn, int chat_id, const char *user_id, int is_admin) {
    if (!conn || !user_id) {
        fprintf(stderr, "Invalid parameters for add_chat_member\n");
        return -1;
    }

    const char *query =
        "INSERT INTO chat_members (chat_id, user_id, is_admin) "
        "VALUES ($1, $2, $3)";

    const char *paramValues[3];
    char chat_id_str[12];
    char is_admin_str[2];

    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    snprintf(is_admin_str, sizeof(is_admin_str), "%d", is_admin);

    paramValues[0] = chat_id_str;
    paramValues[1] = user_id;
    paramValues[2] = is_admin_str;

    PGresult *res = PQexecParams(conn, query, 3, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Error adding chat member: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
