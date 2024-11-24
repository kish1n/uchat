#include "messages.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_message(PGconn *conn, int chat_id, int sender_id, const char *content) {
    if (!conn || !content) {
        fprintf(stderr, "Invalid parameters for create_message\n");
        return -1;
    }

    const char *query = "INSERT INTO messages (chat_id, sender_id, content, sent_at) VALUES ($1, $2, $3, CURRENT_TIMESTAMP);";
    const char *paramValues[3] = {NULL, NULL, NULL};

    char chat_id_str[12];
    char sender_id_str[12];

    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    snprintf(sender_id_str, sizeof(sender_id_str), "%d", sender_id);

    paramValues[0] = chat_id_str;
    paramValues[1] = sender_id_str;
    paramValues[2] = content;

    PGresult *res = PQexecParams(conn, query, 3, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_db_error(conn, "Error creating message");
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}
