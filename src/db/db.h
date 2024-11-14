#ifndef DB_H
#define DB_H

#include <libpq-fe.h>
#include "../config/config.h"

// Функция для подключения к базе данных
PGconn* connect_db(const char *connection_string);

// Функция для закрытия соединения с базой данных
void disconnect_db(PGconn *conn);

#endif // DB_H
