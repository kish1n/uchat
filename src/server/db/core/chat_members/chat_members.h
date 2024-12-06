#ifndef CHAT_MEMBERS_H
#define CHAT_MEMBERS_H

#include <libpq-fe.h>

typedef struct {
    int chat_id;
    char user_id[37];
    int is_admin;
    char joined_at[64];
} ChatMember;

int add_chat_member(PGconn *conn, int chat_id, const char *user_id, int is_admin);
int remove_chat_member(PGconn *conn, int chat_id, const char *user_id);
int get_chat_members(PGconn *conn, int chat_id, ChatMember **members, int *member_count);
int is_user_in_chat(PGconn *conn, int chat_id, const char *user_id);
int is_user_admin(PGconn *conn, int chat_id, const char *user_id);
int delete_user_from_chat(PGconn *conn, int chat_id, const char *user_id);

#endif // CHAT_MEMBERS_H