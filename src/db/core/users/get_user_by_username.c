#include "users.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../../pkg/config/config.h"

User* get_user_by_username(PGconn *conn, const char *username) {
    const char *query = "SELECT uuid, username, passhash, created_at FROM users WHERE username = $1;";
    const char *paramValues[1] = {username};

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        log_db_error(conn, "Error fetching user");
        PQclear(res);
        return NULL;
    }

    if (PQntuples(res) == 0) {
        log_db_error(conn, "No user found with username");
        PQclear(res);
        return NULL;
    }

    User *user = malloc(sizeof(User));
    if (!user) {
        log_db_error(conn, "Failed to allocate memory for user");
        PQclear(res);
        return NULL;
    }

    user->uuid = strdup(PQgetvalue(res, 0, 0));
    user->username = strdup(PQgetvalue(res, 0, 1));
    user->passhash = strdup(PQgetvalue(res, 0, 2));
    user->created_at = strdup(PQgetvalue(res, 0, 3));

    PQclear(res);
    return user;
}
