#ifndef WS_H
#define WS_H

#include <libwebsockets.h>

static Chat *active_chats = NULL;

typedef struct Client {
    struct lws *wsi;
    struct Client *next;
} Client;

typedef struct Chat {
    int chat_id;
    Client *clients;
    struct Chat *next;
} Chat;

void add_client(int chat_id, struct lws *wsi);
void remove_client(struct lws *wsi);
void broadcast_message(int chat_id, const char *message);

#endif // WS_H
