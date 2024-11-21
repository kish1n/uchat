#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "src/services/service.h"       // Ваш заголовочный файл для структуры Server
#include "src/db/core/core.h"        // Заголовочный файл для работы с базой данных

#define PORT 8080

int main() {
    Config config;
    if (load_config("../config.yaml", &config) != 0) {
        fprintf(stderr, "Failed to load config\n");
        return EXIT_FAILURE;
    }

    PGconn *db_conn = connect_db(config.database.url);
    if (db_conn == NULL || PQstatus(db_conn) != CONNECTION_OK) {
        fprintf(stderr, "Failed to connect to database: %s\n", PQerrorMessage(db_conn));
        if (db_conn) disconnect_db(db_conn);
        return EXIT_FAILURE;
    }

    Server *server = server_init(PORT, db_conn);
    if (!server) {
        disconnect_db(db_conn);
        return EXIT_FAILURE;
    }

    if (server_start(server) != 0) {
        fprintf(stderr, "Failed to start server\n");
        server_destroy(server);
        disconnect_db(db_conn);
        return EXIT_FAILURE;
    }

    printf("Server is running on port %d\n", PORT);
    printf("Press Enter to stop the server...\n");

    getchar();

    server_destroy(server);
    disconnect_db(db_conn);

    return EXIT_SUCCESS;
}
