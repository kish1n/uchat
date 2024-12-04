#pragma once
#include "../../../pkg/config/config.h"
#include <libpq-fe.h>

typedef struct {
    int id;
    int chat_id;
    char sender_id[37]; // UUID представляется как строка длиной 36 + 1 для null-терминатора
    char content[1024];
    char sent_at[64];
} Message;

int create_message(PGconn *conn, int chat_id, const char *sender_id, const char *content);
int delete_message(PGconn *conn, int message_id);
int get_message_by_id(PGconn *conn, int message_id, Message *message);
int get_messages_by_chat(PGconn *conn, int chat_id, Message **messages, int *message_count);
int edit_message(PGconn *conn, int message_id, const char *new_content);
int message_exists(PGconn *conn, int message_id);
