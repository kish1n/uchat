#include "messages.h"


int edit_message(PGconn *conn, int message_id, const char *new_content) {
    if (!conn || !new_content) {
        fprintf(stderr, "Invalid parameters for edit_message\n");
        return -1;
    }

    const char *query = "UPDATE messages SET content = $1 WHERE id = $2;";
    const char *paramValues[2] = {NULL, NULL};

    char message_id_str[12];
    snprintf(message_id_str, sizeof(message_id_str), "%d", message_id);

    paramValues[0] = new_content;
    paramValues[1] = message_id_str;

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error editing message");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
