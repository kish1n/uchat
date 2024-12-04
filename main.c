#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "src/server/services/service.h"
#include "src/server/db/core/core.h"
#include "src/server/pkg/config/config.h"
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./uchat_server <port>\n");
        return EXIT_FAILURE;
    }

    char *endptr = NULL;
    int port = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Port must be an integer\n");
        return EXIT_FAILURE;
    }

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Error: Port must be an integer between 1 and 65535.\n");
        return EXIT_FAILURE;
    }

    Config config;
    if (load_config("config.yaml", &config) != 0) {
        logging(ERROR, "Failed to load config");
        return EXIT_FAILURE;
    }

    init_logger(&config.logging);
    logging(INFO, "Server is starting...");

    daemonize();

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    PGconn *db_conn = connect_db(config.database.url);
    if (db_conn == NULL || PQstatus(db_conn) != CONNECTION_OK) {
        logging(ERROR, "Failed to connect to database: %s", PQerrorMessage(db_conn));
        if (db_conn) disconnect_db(db_conn);
        close_logger();
        return EXIT_FAILURE;
    }
    logging(INFO, "Database connection established successfully");

    Server *server = server_init(port, db_conn);
    if (!server) {
        logging(ERROR, "Failed to initialize server");
        disconnect_db(db_conn);
        close_logger();
        return EXIT_FAILURE;
    }

    if (server_start(server, &config) != 0) {
        logging(ERROR, "Failed to start server");
        server_destroy(server);
        disconnect_db(db_conn);
        close_logger();
        return EXIT_FAILURE;
    }

    logging(INFO, "Server is running on port %d", port);
    logging(INFO, "Press Enter to stop the server...");

    while(1) pause();

    logging(INFO, "Server is shutting down...");
    server_destroy(server);
    disconnect_db(db_conn);
    close_logger();

    logging(INFO, "Server stopped");
    return EXIT_SUCCESS;
}