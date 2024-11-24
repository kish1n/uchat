#include "chats.h"
#include "../../../pkg/config/config.h"

int update_chat_name(PGconn *conn, int chat_id, const char *new_name) {
    if (!conn || !new_name) {
        fprintf(stderr, "Invalid parameters for update_chat_name\n");
        return -1;
    }

    const char *query = "UPDATE chats SET name = $1 WHERE id = $2;";
    const char *paramValues[2] = {new_name, NULL};

    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[1] = chat_id_str;

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error updating chat name");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
