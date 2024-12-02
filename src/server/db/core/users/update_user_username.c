#include "users.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int update_user_username(PGconn *conn, const char *username, const char *new_username) {
    const char *query = "UPDATE users SET username = $1 WHERE username = $2;";
    const char *paramValues[2] = {new_username, username};

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error updating user username");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}