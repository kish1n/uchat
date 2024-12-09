#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../pkg/config/config.h" // Ваш логгер

sqlite3 *db = NULL; // Глобальная переменная для хранения соединения с базой данных

int init_db(const char *db_file) {
    int rc = sqlite3_open_v2("uchat.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(db));
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
