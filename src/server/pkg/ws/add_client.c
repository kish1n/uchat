#include "ws.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Добавить клиента в чат
void add_client(int chat_id, struct lws *wsi) {
    Chat *chat = active_chats;

    // Найти чат
    while (chat) {
        if (chat->chat_id == chat_id) {
            break;
        }
        chat = chat->next;
    }

    // Если чат не найден, создаем новый
    if (!chat) {
        chat = malloc(sizeof(Chat));
        chat->chat_id = chat_id;
        chat->clients = NULL;
        chat->next = active_chats;
        active_chats = chat;
    }

    // Добавить клиента
    Client *new_client = malloc(sizeof(Client));
    new_client->wsi = wsi;
    new_client->next = chat->clients;
    chat->clients = new_client;

    printf("Client added to chat_id: %d\n", chat_id);
}

