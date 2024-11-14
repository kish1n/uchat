#ifndef MIGRATIONS_H
#define MIGRATIONS_H

#include <libpq-fe.h>

// Функция для выполнения миграции
int execute_migration(PGconn *conn, const char *filename);

#endif // MIGRATIONS_H
