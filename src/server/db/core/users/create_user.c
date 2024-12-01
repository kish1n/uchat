#include "users.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int create_user(PGconn *conn, const char *username, const char *passhash) {
    const char *query = "INSERT INTO users (id, username, passhash, created_at) VALUES (gen_random_uuid(), $1, $2, CURRENT_TIMESTAMP);";
    const char *paramValues[2] = {username, passhash};

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error creating user");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}