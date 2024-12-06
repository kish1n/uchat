#include <stdio.h>
#include <sqlite3.h>
#include "chat_members.h"

int delete_user_from_chat(sqlite3 *db, int chat_id, const char *user_id) {
    return remove_chat_member(db, chat_id, user_id);
}
