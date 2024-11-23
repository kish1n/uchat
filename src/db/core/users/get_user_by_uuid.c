#include "users.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

User* get_user_by_uuid(PGconn *conn, const char *uuid) {
    const char *query = "SELECT uuid, username, passhash, created_at FROM users WHERE uuid = $1;";
    const char *paramValues[1] = {uuid};

    // Выполняем запрос
    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error fetching user: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return NULL;
    }

    // Проверяем, что пользователь найден
    if (PQntuples(res) == 0) {
        fprintf(stderr, "No user found with UUID: %s\n", uuid);
        PQclear(res);
        return NULL;
    }

    // Создаем и заполняем структуру User
    User *user = malloc(sizeof(User));
    if (!user) {
        fprintf(stderr, "Memory allocation failed\n");
        PQclear(res);
        return NULL;
    }

    // Копируем данные из результата запроса
    user->uuid = strdup(PQgetvalue(res, 0, 0));
    user->username = strdup(PQgetvalue(res, 0, 1));
    user->passhash = strdup(PQgetvalue(res, 0, 2));
    user->created_at = strdup(PQgetvalue(res, 0, 3));

    PQclear(res);
    return user;
}