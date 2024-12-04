#include "chats.h"
#include "../../../pkg/config/config.h"

int delete_chat(PGconn *conn, int chat_id) {
    if (!conn) {
        fprintf(stderr, "Invalid parameters for delete_chat\n");
        return -1;
    }

    const char *query = "DELETE FROM chats WHERE id = $1;";
    const char *paramValues[1] = {NULL};

    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Error deleting chat");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
