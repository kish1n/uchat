#include <stdio.h>
#include <stdlib.h>
#include "db.h"

// Function to connect to the database
PGconn* connect_db(const char *connection_string) {
    PGconn *conn = PQconnectdb(connection_string);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Database connection error: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    printf("Database connection established successfully\n");
    return conn;
}

// Function to close the database connection
void disconnect_db(PGconn *conn) {
    if (conn) {
        PQfinish(conn);
        printf("Database connection closed\n");
    }
}
