#include "users.h"
#include <stdio.h>
#include <stdlib.h>

int create_user(PGconn *conn, const char *username, const char *passhash) {
    const char *query = "INSERT INTO users (uuid, username, passhash, created_at) VALUES (gen_random_uuid(), $1, $2, CURRENT_TIMESTAMP);";
    const char *paramValues[2] = {username, passhash};

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Error creating user: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}

int update_user_password(PGconn *conn, const char *uuid, const char *new_passhash) {
    const char *query = "UPDATE users SET passhash = $1 WHERE uuid = $2;";
    const char *paramValues[2] = {new_passhash, uuid};

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Error updating user password: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}

int update_user_username(PGconn *conn, const char *uuid, const char *new_username) {
    const char *query = "UPDATE users SET username = $1 WHERE uuid = $2;";
    const char *paramValues[2] = {new_username, uuid};

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Error updating user username: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}

int delete_user(PGconn *conn, const char *uuid) {
    const char *query = "DELETE FROM users WHERE uuid = $1;";
    const char *paramValues[1] = {uuid};

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Error deleting user: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
