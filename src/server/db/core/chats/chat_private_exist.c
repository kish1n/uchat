#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

#include "chats.h"

int private_chat_exist(PGconn *conn, const char *user1_id, const char *user2_id) {
    if (!conn || !user1_id || !user2_id) {
        fprintf(stderr, "Invalid parameters passed to private_chat_exist\n");
        return -1;
    }

    const char *query =
        "SELECT c.id "
        "FROM chats c "
        "JOIN chat_members cm1 ON c.id = cm1.chat_id "
        "JOIN chat_members cm2 ON c.id = cm2.chat_id "
        "WHERE cm1.user_id = $1 AND cm2.user_id = $2 AND c.is_group = FALSE";

    const char *paramValues[2];
    paramValues[0] = user1_id;
    paramValues[1] = user2_id;

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Database error: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    int chat_id = 0;
    if (PQntuples(res) > 0) {
        chat_id = atoi(PQgetvalue(res, 0, 0));
    }

    PQclear(res);
    return chat_id;
}
