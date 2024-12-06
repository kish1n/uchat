#ifndef CHAT_MEMBERS_H
#define CHAT_MEMBERS_H

#include <sqlite3.h>

typedef struct {
    int chat_id;
    char user_id[37];   // UUID
    int is_admin;       // Boolean: 0 or 1
    char joined_at[64]; // Timestamp
} ChatMember;

int add_chat_member(sqlite3 *db, int chat_id, const char *user_id, int is_admin);
int remove_chat_member(sqlite3 *db, int chat_id, const char *user_id);
int get_chat_members(sqlite3 *db, int chat_id, ChatMember **members, int *member_count);
int is_user_in_chat(sqlite3 *db, int chat_id, const char *user_id);
int is_user_admin(sqlite3 *db, int chat_id, const char *user_id);
int delete_user_from_chat(sqlite3 *db, int chat_id, const char *user_id);

#endif // CHAT_MEMBERS_H
