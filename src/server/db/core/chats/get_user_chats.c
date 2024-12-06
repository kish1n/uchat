#include "chats.h"
#include <postgresql/libpq-fe.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_user_chats(PGconn *db_conn, const char *user_id) {
    const char *query =
        "SELECT c.id AS chat_id, c.name AS chat_name "
        "FROM chats c "
        "JOIN chat_members cm ON c.id = cm.chat_id "
        "WHERE cm.user_id = $1";

    PGresult *res;
    const char *param_values[1] = {user_id};

    res = PQexecParams(db_conn, query, 1, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Database query failed: %s\n", PQerrorMessage(db_conn));
        PQclear(res);
        return NULL;
    }

    struct json_object *response_array = json_object_new_array();

    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        struct json_object *chat_obj = json_object_new_object();
        json_object_object_add(
            chat_obj, "id",
            json_object_new_string(PQgetvalue(res, i, 0))); // chat_id
        json_object_object_add(
            chat_obj, "name",
            json_object_new_string(PQgetvalue(res, i, 1) ? PQgetvalue(res, i, 1) : "Unnamed Chat")); // chat_name

        json_object_array_add(response_array, chat_obj);
    }

    char *response = strdup(json_object_to_json_string(response_array));

    json_object_put(response_array);
    PQclear(res);

    return response;
}
