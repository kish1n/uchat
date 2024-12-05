#include "ws.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Отправить сообщение всем клиентам в чате
void broadcast_message(int chat_id, const char *message) {
    Chat *chat = active_chats;

    // Найти чат
    while (chat) {
        if (chat->chat_id == chat_id) {
            break;
        }
        chat = chat->next;
    }

    if (!chat) {
        printf("No active clients in chat_id: %d\n", chat_id);
        return;
    }

    // Отправить сообщение всем клиентам
    Client *client = chat->clients;
    while (client) {
        lws_write(client->wsi, (unsigned char *)message, strlen(message), LWS_WRITE_TEXT);
        client = client->next;
    }
}
