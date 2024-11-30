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


#endif // CHATS_H