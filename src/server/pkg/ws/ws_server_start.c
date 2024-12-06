#include "ws.h"

int ws_server_start(int port) {
    struct lws_context_creation_info info;
    struct lws_context *context;
    struct lws_protocols protocols[] = {
        {"http", ws_chat_handler, 0, 0},
        {NULL, NULL, 0, 0}
    };

    memset(&info, 0, sizeof(info));
    info.port = port;
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "Failed to create context\n");
        return -1;
    }

    while (1) {
        lws_service(context, 1000);
    }

    lws_context_destroy(context);
    return 0;
}