#include <stdio.h>
#include "src/config/config.h"
#include "src/db/core/core.h"
#include "src/db/core/migrations.h"
#include "src/logger/logger.h"
#include "src/db/users/users.h"

int main() {
    Config config;
    if (load_config("../config.yaml", &config) != 0) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }

    init_logger(&config);

    log_message(INFO, "starting app...");

    PGconn *conn = connect_db(config.database.url);
    if (!conn) {
        log_message(ERROR, "Error connection to database is confused");
        return 1;
    }

    log_message(INFO, "connection to database is established");

    if (execute_migration(conn, "../src/db/migrations/001_init.sql") != 0) {
        log_message(ERROR, "Error migration");
    } else {
        log_message(INFO, "Migration is successful");
    }

    create_user(conn, "username", "sdakodsdlakfs");

    disconnect_db(conn);
    log_message(INFO, "connection to database is closed");

    return 0;
}
