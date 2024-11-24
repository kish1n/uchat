#include "messages.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int delete_message(PGconn *conn, int message_id) {
    if (!conn) {
        fprintf(stderr, "Invalid connection for delete_message\n");
        return -1;
    }

    const char *query = "DELETE FROM messages WHERE id = $1;";
    const char *paramValues[1] = {NULL};

    char message_id_str[12];
    snprintf(message_id_str, sizeof(message_id_str), "%d", message_id);
    paramValues[0] = message_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error deleting message");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
