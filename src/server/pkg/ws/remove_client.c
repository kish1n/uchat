#include "ws.h"
#include <stdlib.h>
#include <string.h>

// Удаление клиента из массива
void remove_client(struct lws *wsi) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].wsi == wsi) {
            logging(INFO, "Removing client: user_id=%s, chat_id=%s", clients[i].user_id, clients[i].chat_id);

            // Сдвиг всех последующих клиентов в массиве
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }

            client_count--;
            return;
        }
    }

    logging(WARN, "Attempted to remove non-existent client");
}

