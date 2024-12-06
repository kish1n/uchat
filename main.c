#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sqlite3.h>
#include "src/server/services/service.h"
#include "src/server/db/core/core.h"
#include "src/server/pkg/config/config.h"

// Signal handler
volatile sig_atomic_t stop_server = 0;

void signal_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        stop_server = 1;
    }
}

void daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // Parent process exits
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        perror("Failed to create new session");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null < 0) {
        perror("Failed to open /dev/null");
        exit(EXIT_FAILURE);
    }

    dup2(dev_null, STDIN_FILENO);
    dup2(dev_null, STDOUT_FILENO);
    dup2(dev_null, STDERR_FILENO);

    if (dev_null > STDERR_FILENO) {
        close(dev_null);
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

    daemonize();

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

    // WebSocket-related code commented out for now
    // pthread_t ws_thread;
    // int ws_port = 8081;
    // if (pthread_create(&ws_thread, NULL, start_ws_server, &ws_port) != 0) {
    //     logging(ERROR, "Failed to create WebSocket server thread");
    //     close_db();
    //     close_logger();
    //     return EXIT_FAILURE;
    // }

    Server *server = server_init(port, NULL); // Pass NULL for db_conn since it's SQLite now
    if (!server) {
        logging(ERROR, "Failed to initialize server");
        close_db();
        close_logger();
        return EXIT_FAILURE;
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
