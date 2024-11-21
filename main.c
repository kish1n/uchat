#include <stdio.h>
#include <stdlib.h>
#include "src/services/service.h"
#include <microhttpd.h> // Для работы с HTTP
#include <json-c/json.h> // Для работы с JSON

int main() {
    Server *server = server_init(PORT);
    if (!server) {
        return EXIT_FAILURE;
    }

    if (server_start(server) != 0) {
        server_destroy(server);
        return EXIT_FAILURE;
    }

    printf("Press Enter to stop the server...\n");
    getchar();

    server_destroy(server);
    return EXIT_SUCCESS;
}