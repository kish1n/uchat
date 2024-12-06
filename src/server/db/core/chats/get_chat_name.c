#include <stdlib.h>
#include <string.h>

#include "chats.h"
#include "../../../pkg/config/config.h"

int get_chat_name(PGconn *conn, int chat_id, char *chat_name, size_t chat_name_size) {
    const char *query = "SELECT name FROM chats WHERE id = $1";
    const char *paramValues[1];
    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return -1;
    }

    strncpy(chat_name, PQgetvalue(res, 0, 0), chat_name_size - 1);
    chat_name[chat_name_size - 1] = '\0';
    PQclear(res);
    return 0;
}
