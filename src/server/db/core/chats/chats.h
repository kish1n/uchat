#ifndef CHATS_H
#define CHATS_H

#include <sqlite3.h>
#include <stddef.h>
#include <stdbool.h>
#include "../../../pkg/config/config.h"

typedef struct {
    int id;
    char name[255];
    int is_group;
    int last_message_id;
    char created_at[64];
} Chat;

int create_chat(sqlite3 *db, const char *chat_name, int is_group);
int update_chat_name(sqlite3 *db, int chat_id, const char *new_name);
int delete_chat(sqlite3 *db, int chat_id);
int get_chat_by_id(sqlite3 *db, int chat_id, Chat *chat);
int chat_exists(sqlite3 *db, int chat_id);
int is_chat_group(sqlite3 *db, int chat_id);
int private_chat_exist(sqlite3 *db, const char *user1_id, const char *user2_id);
int get_chat_name(sqlite3 *db, int chat_id, char *chat_name, size_t chat_name_size);
char *get_chat_messages(sqlite3 *db, int chat_id, Config *cfg);
char *get_user_chats(sqlite3 *db, const char *user_id);
int update_last_message_id(sqlite3 *db, int chat_id, int message_id);
int get_last_message_id_in_chat(sqlite3 *db, int chat_id);
int get_last_message_in_chat(sqlite3 *db, int chat_id);

#endif // CHATS_H
