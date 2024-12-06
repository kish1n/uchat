#include "chats.h"
#include <postgresql/libpq-fe.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_chat_messages(PGconn *db_conn, int chat_id) {
    const char *query =
        "SELECT m.id AS message_id, u.username AS sender, m.content AS message, m.sent_at "
        "FROM messages m "
        "JOIN users u ON m.sender_id = u.id "
        "WHERE m.chat_id = $1 "
        "ORDER BY m.sent_at ASC";

    PGresult *res;
    const char *param_values[1];
    char chat_id_str[12]; // Для хранения chat_id в виде строки
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
    param_values[0] = chat_id_str;

    res = PQexecParams(db_conn, query, 1, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Database query failed: %s\n", PQerrorMessage(db_conn));
        PQclear(res);
        return NULL;
    }

    struct json_object *response_array = json_object_new_array();

    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        struct json_object *message_obj = json_object_new_object();
        json_object_object_add(
            message_obj, "id",
            json_object_new_string(PQgetvalue(res, i, 0))); // message_id
        json_object_object_add(
            message_obj, "sender",
            json_object_new_string(PQgetvalue(res, i, 1))); // sender
        json_object_object_add(
            message_obj, "message",
            json_object_new_string(PQgetvalue(res, i, 2))); // content
        json_object_object_add(
            message_obj, "sent_at",
            json_object_new_string(PQgetvalue(res, i, 3))); // sent_at

        json_object_array_add(response_array, message_obj);
    }

    char *response = strdup(json_object_to_json_string(response_array));

    json_object_put(response_array);
    PQclear(res);

    return response;
}
