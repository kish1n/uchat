#include "users.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int update_user_password(PGconn *conn, const char *uuid, const char *new_passhash) {
    const char *query = "UPDATE users SET passhash = $1 WHERE id = $2;";
    const char *paramValues[2] = {new_passhash, uuid};

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error updating user password");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}