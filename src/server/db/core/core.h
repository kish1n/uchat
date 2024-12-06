#ifndef DB_H
#define DB_H

#include <libpq-fe.h>
#include "../../pkg/config/config.h"

PGconn* connect_db(const char *connection_string);
void disconnect_db(PGconn *conn);
int execute_migration(PGconn *conn, const char *filename);

#endif // DB_H
