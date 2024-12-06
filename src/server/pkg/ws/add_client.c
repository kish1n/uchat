#include "ws.h"

Client clients[MAX_CLIENTS] = {0}; // Инициализация массива клиентов
int client_count = 0; // Инициализация счетчика клиентов

void add_client(const char *chat_id, const char *user_id, struct lws *wsi) {
    if (client_count >= MAX_CLIENTS) {
        logging(WARN, "Maximum number of clients reached. Connection rejected.");
        return;
    }

    strncpy(clients[client_count].chat_id, chat_id, sizeof(clients[client_count].chat_id) - 1);
    strncpy(clients[client_count].user_id, user_id, sizeof(clients[client_count].user_id) - 1);
    clients[client_count].wsi = wsi;

    client_count++;
    logging(INFO, "Client added: user_id=%s, chat_id=%s, current_client_count=%d", user_id, chat_id, client_count);
}
