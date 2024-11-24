#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "src/services/service.h"
#include "src/db/core/core.h"
#include "src/pkg/config/config.h"

#define PORT 8080

int main() {
    Config config;
    if (load_config("../config.yaml", &config) != 0) {
        logging(ERROR, "Failed to load config");
        return EXIT_FAILURE;
    }

    init_logger(&config.logging);
    logging(INFO, "Server is starting...");

    PGconn *db_conn = connect_db(config.database.url);
    if (db_conn == NULL || PQstatus(db_conn) != CONNECTION_OK) {
        logging(ERROR, "Failed to connect to database: %s", PQerrorMessage(db_conn));
        if (db_conn) disconnect_db(db_conn);
        close_logger();
        return EXIT_FAILURE;
    }
    logging(INFO, "Database connection established successfully");

    Server *server = server_init(PORT, db_conn);
    if (!server) {
        logging(ERROR, "Failed to initialize server");
        disconnect_db(db_conn);
        close_logger();
        return EXIT_FAILURE;
    }

    if (server_start(server) != 0) {
        logging(ERROR, "Failed to start server");
        server_destroy(server);
        disconnect_db(db_conn);
        close_logger();
        return EXIT_FAILURE;
    }

    logging(INFO, "Server is running on port %d", PORT);
    logging(INFO, "Press Enter to stop the server...");

    getchar();

    logging(INFO, "Server is shutting down...");
    server_destroy(server);
    disconnect_db(db_conn);
    close_logger();

    logging(INFO, "Server stopped");
    return EXIT_SUCCESS;
}
