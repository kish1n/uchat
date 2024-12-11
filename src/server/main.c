#include "server.h"
#include <stdio.h>
#include <stdlib.h>  // Для Server, server_init, server_start, server_destroy
#include "db/core/core.h" // Для init_db, create_tables, close_db
#include "services/service.h"
#include <sodium.h>

volatile sig_atomic_t stop_server;


void signal_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        stop_server = 1;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number: %d\n", port);
        return EXIT_FAILURE;
    }

    //daemonize();


    Config config;
    if (load_config("config.yaml", &config) != 0) {
        fprintf(stderr, "Failed to load config\n");
        return EXIT_FAILURE;
    }

    init_logger(&config.logging);
    logging(INFO, "Server is starting...");

    // Initialize SQLite database
    if (init_db("uchat.db") != 0) {
        logging(ERROR, "Failed to initialize SQLite database");
        close_logger();
        return EXIT_FAILURE;
    }
    logging(INFO, "SQLite database initialized successfully");

    if (create_tables() != 0) {
        logging(ERROR, "Failed to create tables in SQLite database");
        close_db();
        close_logger();
        return EXIT_FAILURE;
    }
    logging(INFO, "Database tables created successfully");

    Server *server = server_init(port); // Pass NULL for db_conn since it's SQLite now
    if (!server) {
        logging(ERROR, "Failed to initialize server");
        close_db();
        close_logger();
        return EXIT_FAILURE;
    }

    if (sodium_init() == -1) {
        fprintf(stderr, "Failed to initialize libsodium\n");
        exit(EXIT_FAILURE);
    }

    if (server_start(server) != 0) {
        logging(ERROR, "Failed to start server");
        server_destroy(server);
        close_db();
        close_logger();
        return EXIT_FAILURE;
    }
    logging(INFO, "Server is running on port %d", port);

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    while (!stop_server) {
        sleep(1);
    }

    logging(INFO, "Server is shutting down...");
    server_destroy(server);
    close_db();
    close_logger();

    logging(INFO, "Server stopped");
    return EXIT_SUCCESS;
}
