#include "users.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>

int delete_user(PGconn *conn, const char *uuid) {
    const char *query = "DELETE FROM users WHERE id = $1;";
    const char *paramValues[1] = {uuid};

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error deleting user");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
