#include "core.h"

int execute_sql(const char *sql) {
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        logging(ERROR, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }
    logging(INFO, "SQL executed successfully.");
    return 0;
}