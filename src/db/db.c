#include <stdio.h>
#include <stdlib.h>
#include "db.h"

// Функция для подключения к базе данных
PGconn* connect_db(const char *connection_string) {
    PGconn *conn = PQconnectdb(connection_string);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Ошибка подключения к базе данных: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    printf("Подключение к базе данных успешно установлено\n");
    return conn;
}

// Функция для закрытия соединения с базой данных
void disconnect_db(PGconn *conn) {
    if (conn) {
        PQfinish(conn);
        printf("Соединение с базой данных закрыто\n");
    }
}
