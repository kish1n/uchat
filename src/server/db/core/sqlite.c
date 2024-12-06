#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../pkg/config/config.h" // Ваш логгер

sqlite3 *db = NULL; // Глобальная переменная для хранения соединения с базой данных

int init_db(const char *db_file) {
    int rc = sqlite3_open(db_file, &db);
    if (rc) {
        logging(ERROR, "Can't open database: %s", sqlite3_errmsg(db));
        return -1;
    }
    logging(INFO, "Opened database successfully: %s", db_file);
    return 0;
}

sqlite3 *get_db() {
    return db;
}

void close_db() {
    if (db) {
        sqlite3_close(db);
        db = NULL;
        logging(INFO, "Database connection closed.");
    }
}
