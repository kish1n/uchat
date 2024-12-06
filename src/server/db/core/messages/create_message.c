#include "messages.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_message(PGconn *conn, int chat_id, const char *sender_id, const char *content) {
    if (!conn || chat_id <= 0 || !sender_id || !content) {
        fprintf(stderr, "Invalid parameters for create_message\n");
        return -1;
    }

    const char *query =
        "INSERT INTO messages (chat_id, sender_id, content, sent_at) "
        "VALUES ($1, $2::uuid, $3, NOW()) RETURNING id";

    const char *paramValues[3];
    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    paramValues[0] = chat_id_str;
    paramValues[1] = sender_id;
    paramValues[2] = content;

    PGresult *res = PQexecParams(conn, query, 3, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error creating message: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    int message_id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return message_id;
}