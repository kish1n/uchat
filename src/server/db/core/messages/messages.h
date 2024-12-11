#pragma once

#include "../../../pkg/config/config.h"
#include <sqlite3.h>

typedef struct {
    int id;
    int chat_id;
    char sender_id[37]; // UUID представляется как строка длиной 36 + 1 для null-терминатора
    char content[1024];
    char sent_at[64];
} Message;

int create_message(sqlite3 *db, int chat_id, const char *sender_id, const char *content, Config *cfg);
int delete_message(sqlite3 *db, int message_id);
int get_message_by_id(sqlite3 *db, int message_id, Message *message, Config *cfg);
int get_messages_by_chat(sqlite3 *db, int chat_id, Message **messages, int *message_count);
int edit_message(sqlite3 *db, int message_id, const char *new_content, Config *cfg);
int message_exists(sqlite3 *db, int message_id);
int get_new_messages_by_chat(sqlite3 *db, int chat_id, int last_message_id, Message **messages, int *message_count);
int get_last_message_id(sqlite3 *db, int chat_id);
int edit_last_message_id(sqlite3 *db, int chat_id, int message_id);
int delete_all_messages_in_chat(sqlite3 *db, int chat_id);
int get_chat_id_by_message_id(sqlite3 *db, int message_id);