#include <stdio.h>
#include <stdlib.h>
#include "core.h"

PGconn* connect_db(const char *connection_string) {
    PGconn *conn = PQconnectdb(connection_string);

    if (PQstatus(conn) != CONNECTION_OK) {
        PQfinish(conn);
        return NULL;
    }

    return conn;
}
void disconnect_db(PGconn *conn) {
    if (conn) {
        PQfinish(conn);
    }
}

