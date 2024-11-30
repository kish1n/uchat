#include <stdlib.h>
#include <string.h>

#include "chats.h"
#include "../../../pkg/config/config.h"

int get_chat_by_id(PGconn *conn, int chat_id, Chat *chat) {
    if (!conn || !chat) {
        fprintf(stderr, "Invalid parameters for get_chat_by_id\n");
        return -1;
    }

    const char *query = "SELECT id, name, is_group, created_at FROM chats WHERE id = $1;";
    const char *paramValues[1] = {NULL};

    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        log_db_error(conn, "Error fetching chat by id");
        PQclear(res);
        return -1;
    }

    if (PQntuples(res) == 0) {
        fprintf(stderr, "Chat with id %d not found\n", chat_id);
        PQclear(res);
        return -1;
    }

    chat->id = atoi(PQgetvalue(res, 0, 0));
    strncpy(chat->name, PQgetvalue(res, 0, 1), sizeof(chat->name) - 1);
    chat->name[sizeof(chat->name) - 1] = '\0';
    chat->is_group = strcmp(PQgetvalue(res, 0, 2), "t") == 0 ? 1 : 0;
    strncpy(chat->created_at, PQgetvalue(res, 0, 3), sizeof(chat->created_at) - 1);
    chat->created_at[sizeof(chat->created_at) - 1] = '\0';

    PQclear(res);
    return 0;
}
