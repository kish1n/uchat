#include "messages.h"

int message_exists(PGconn *conn, int message_id) {
    if (!conn || message_id <= 0) {
        fprintf(stderr, "Invalid parameters for message_exists\n");
        return 0;
    }

    const char *query =
        "SELECT 1 FROM messages WHERE id = $1";
    const char *paramValues[1];
    char message_id_str[12];
    snprintf(message_id_str, sizeof(message_id_str), "%d", message_id);
    paramValues[0] = message_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error checking if message exists: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    int exists = PQntuples(res) > 0 ? 1 : 0;

    PQclear(res);
    return exists;
}
