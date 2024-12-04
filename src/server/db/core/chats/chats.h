#ifndef CHATS_H
#define CHATS_H

#include <libpq-fe.h>

typedef struct {
    int id;
    char name[255];
    int is_group;
    char created_at[64];
} Chat;

int create_chat(PGconn *conn, const char *chat_name, int is_group);
int update_chat_name(PGconn *conn, int chat_id, const char *new_name);
int delete_chat(PGconn *conn, int chat_id);
int get_chat_by_id(PGconn *conn, int chat_id, Chat *chat);
int chat_exists(PGconn *conn, int chat_id);
int is_chat_group(PGconn *conn, int chat_id);
int private_chat_exist(PGconn *conn, const char *user1_id, const char *user2_id);
int get_chat_name(PGconn *conn, int chat_id, char *chat_name, size_t chat_name_size);

#endif // CHATS_H