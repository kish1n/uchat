#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include "../../pkg/config/config.h"

extern sqlite3 *db;

int create_tables();
int execute_sql(const char *sql);
void generate_uuid(char *uuid_str);
int init_db(const char *db_file);
void close_db();
sqlite3 *get_db();

#endif // DB_H
