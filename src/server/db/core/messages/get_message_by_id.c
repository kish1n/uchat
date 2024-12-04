#include "messages.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_message_by_id(PGconn *conn, int message_id, Message *message) {
    if (!conn || message_id <= 0 || !message) {
        fprintf(stderr, "Invalid parameters for get_message_by_id\n");
        return -1;
    }

    const char *query =
        "SELECT id, chat_id, sender_id, content, sent_at "
        "FROM messages WHERE id = $1";
    const char *paramValues[1];
    char message_id_str[12];
    snprintf(message_id_str, sizeof(message_id_str), "%d", message_id);
    paramValues[0] = message_id_str;

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error fetching message: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    if (PQntuples(res) == 0) {
        fprintf(stderr, "Message not found: ID %d\n", message_id);
        PQclear(res);
        return -1;
    }

    message->id = atoi(PQgetvalue(res, 0, 0));
    message->chat_id = atoi(PQgetvalue(res, 0, 1));
    strncpy(message->sender_id, PQgetvalue(res, 0, 2), sizeof(message->sender_id) - 1);
    message->sender_id[sizeof(message->sender_id) - 1] = '\0';
    strncpy(message->content, PQgetvalue(res, 0, 3), sizeof(message->content) - 1);
    message->content[sizeof(message->content) - 1] = '\0';
    strncpy(message->sent_at, PQgetvalue(res, 0, 4), sizeof(message->sent_at) - 1);
    message->sent_at[sizeof(message->sent_at) - 1] = '\0';

    PQclear(res);
    return 0;
}
