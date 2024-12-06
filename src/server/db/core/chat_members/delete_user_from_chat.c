#include "chat_members.h"
#include <stdlib.h>
#include <stdio.h>

int delete_user_from_chat(PGconn *conn, int chat_id, const char *user_id) {
    if (!conn || !user_id) {
        fprintf(stderr, "Invalid parameters for delete_user_from_chat\n");
        return -1; // Возвращаем ошибку
    }

    const char *query =
        "DELETE FROM chat_members "
        "WHERE chat_id = $1 AND user_id::uuid = $2";

    const char *paramValues[2];
    char chat_id_str[12];
    snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);

    paramValues[0] = chat_id_str;
    paramValues[1] = user_id;

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Error deleting user from chat: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1; // Возвращаем ошибку
    }

    int affected_rows = atoi(PQcmdTuples(res)); // Проверяем, сколько строк было удалено
    PQclear(res);

    if (affected_rows == 0) {
        fprintf(stderr, "No such user in chat with ID %d\n", chat_id);
        return 0; // Пользователь не найден в чате
    }

    return 1; // Успешное удаление
}
