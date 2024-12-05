#include "ws.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Удалить клиента из всех чатов
void remove_client(struct lws *wsi) {
    Chat *chat = active_chats;

    while (chat) {
        Client **current = &chat->clients;

        while (*current) {
            if ((*current)->wsi == wsi) {
                Client *to_remove = *current;
                *current = (*current)->next;
                free(to_remove);
                printf("Client removed from chat_id: %d\n", chat->chat_id);
                return;
            }
            current = &((*current)->next);
        }

        chat = chat->next;
    }
}

