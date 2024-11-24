#include "messages.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_messages_by_chat(PGconn *conn, int chat_id, Message **messages, int *message_count) {
    if (!conn || !messages || !message_count) {
        fprintf(stderr, "Invalid parameters for get_messages_by_chat\n");
        return -1;
    }

    const char *query = "SELECT id, chat_id, sender_id, content, sent_at FROM messages WHERE chat_id = $1 ORDER BY sent_at ASC;";
    const char *paramValues[1] = {NULL};

    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        log_db_error(conn, "Error fetching messages by chat");
        PQclear(res);
        return -1;
    }

    int rows = PQntuples(res);
    *messages = malloc(rows * sizeof(Message));
    *message_count = rows;

    for (int i = 0; i < rows; i++) {
        (*messages)[i].id = atoi(PQgetvalue(res, i, 0));
        (*messages)[i].chat_id = atoi(PQgetvalue(res, i, 1));
        (*messages)[i].sender_id = atoi(PQgetvalue(res, i, 2));
        strncpy((*messages)[i].content, PQgetvalue(res, i, 3), sizeof((*messages)[i].content) - 1);
        (*messages)[i].content[sizeof((*messages)[i].content) - 1] = '\0';
        strncpy((*messages)[i].sent_at, PQgetvalue(res, i, 4), sizeof((*messages)[i].sent_at) - 1);
        (*messages)[i].sent_at[sizeof((*messages)[i].sent_at) - 1] = '\0';
    }

    PQclear(res);
    return 0;
}
