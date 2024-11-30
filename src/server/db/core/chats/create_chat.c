#include "chats.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int create_chat(PGconn *conn, const char *chat_name, int is_group) {
    if (conn == NULL || chat_name == NULL) {
        fprintf(stderr, "Invalid parameters for create_chat\n");
        return -1;
    }

    const char *query = "INSERT INTO chats (name, is_group, created_at) VALUES ($1, $2, CURRENT_TIMESTAMP);";

    const char *paramValues[2];
    paramValues[0] = chat_name;
    paramValues[1] = is_group ? "TRUE" : "FALSE";

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error creating chat");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
