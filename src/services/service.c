#include <stdlib.h>
#include <microhttpd.h>
#include "service.h"
#include "../db/core/core.h"
#include "auth/auth_handlers.h"

Server* server_init(int port, PGconn *db_conn) {
    Server *server = malloc(sizeof(Server));
    if (!server) {
        fprintf(stderr, "Failed to allocate memory for server\n");
        return NULL;
    }
    server->port = port;
    server->daemon = NULL;
    server->db_conn = db_conn; // Сохраняем соединение с базой данных
    return server;
}


int server_start(Server *server) {
    server->daemon = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY, server->port, NULL, NULL,
        &handle_request, NULL, MHD_OPTION_NOTIFY_COMPLETED, &free_request_data, NULL, MHD_OPTION_END);
    if (!server->daemon) {
        fprintf(stderr, "Failed to start server on port %d\n", server->port);
        return -1;
    }

    printf("Server is running on port %d\n", server->port);
    return 0;
}

void server_stop(Server *server) {
    if (server->daemon) {
        MHD_stop_daemon(server->daemon);
        server->daemon = NULL;
        printf("Server stopped\n");
    }
}

void server_destroy(Server *server) {
    if (server) {
        server_stop(server);
        free(server);
    }
}