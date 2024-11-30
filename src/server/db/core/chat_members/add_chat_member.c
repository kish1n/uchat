#include "chat_members.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int add_chat_member(PGconn *conn, int chat_id, int user_id, int is_admin) {
    if (!conn) {
        fprintf(stderr, "Invalid connection for add_chat_member\n");
        return -1;
    }

    const char *query = "INSERT INTO chat_members (chat_id, user_id, is_admin, joined_at) VALUES ($1, $2, $3, CURRENT_TIMESTAMP);";
    const char *paramValues[3] = {NULL, NULL, NULL};

    char chat_id_str[12];
    char user_id_str[12];
    char is_admin_str[6];

    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);
    snprintf(is_admin_str, sizeof(is_admin_str), "%s", is_admin ? "TRUE" : "FALSE");

    paramValues[0] = chat_id_str;
    paramValues[1] = user_id_str;
    paramValues[2] = is_admin_str;

    PGresult *res = PQexecParams(conn, query, 3, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error adding member to chat");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
