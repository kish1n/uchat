#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>


int create_chat(PGconn *conn, const char *name, int is_group) {
    if (!conn || !name) {
        fprintf(stderr, "Invalid parameters for create_chat\n");
        return -1;
    }

    const char *query = "INSERT INTO chats (name, is_group, created_at) VALUES ($1, $2, CURRENT_TIMESTAMP) RETURNING id;";
    const char *paramValues[2] = {name, NULL};
    char is_group_str[2]; // Для преобразования `is_group` в строку
    snprintf(is_group_str, sizeof(is_group_str), "%d", is_group);
    paramValues[1] = is_group_str;

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error inserting chat: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    // Получаем id созданного чата
    char *id_str = PQgetvalue(res, 0, 0);
    int chat_id = atoi(id_str);

    PQclear(res);
    return chat_id;
}
