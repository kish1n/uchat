#ifndef WS_H
#define WS_H

#include <libwebsockets.h>
#include "../config/config.h"
#include <json-c/json.h>
#include <string.h>

#define MAX_CLIENTS 100
#define MAX_MESSAGE_SIZE 1024 // 1 КБ для текстовых сообщений


typedef struct Client {
    char chat_id[64];
    char user_id[64];
    struct lws *wsi;
} Client;

extern Client clients[MAX_CLIENTS]; // Глобальный массив клиентов
extern int client_count; // Глобальный счетчик клиентов

void add_client(const char *chat_id, const char *user_id, struct lws *wsi);
void remove_client(struct lws *wsi);
void broadcast_message(const char *chat_id, const char *message);
int ws_server_start(int port);
int ws_chat_handler(struct lws *wsi, enum lws_callback_reasons reason,
                    void *user, void *in, size_t len);

#endif // WS_H
