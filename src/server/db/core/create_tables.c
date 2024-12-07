#include "core.h"

int create_tables() {
    const char *create_users =
        "CREATE TABLE IF NOT EXISTS users ("
        "id TEXT PRIMARY KEY NOT NULL, "
        "username TEXT UNIQUE NOT NULL, "
        "passhash TEXT NOT NULL, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    const char *create_chats =
        "CREATE TABLE IF NOT EXISTS chats ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT, "
        "is_group BOOLEAN NOT NULL DEFAULT 0, "
        "last_message_id INTEGER DEFAULT NULL, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (last_message_id) REFERENCES messages (id) ON DELETE SET NULL"
        ");";

    const char *create_chat_members =
        "CREATE TABLE IF NOT EXISTS chat_members ("
        "chat_id INTEGER NOT NULL, "
        "user_id TEXT NOT NULL, "
        "is_admin BOOLEAN NOT NULL DEFAULT 0, "
        "joined_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "PRIMARY KEY (chat_id, user_id), "
        "FOREIGN KEY (chat_id) REFERENCES chats (id) ON DELETE CASCADE, "
        "FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE"
        ");";

    const char *create_messages =
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "chat_id INTEGER NOT NULL, "
        "sender_id TEXT NOT NULL, "
        "content TEXT NOT NULL, "
        "sent_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (chat_id) REFERENCES chats (id) ON DELETE CASCADE, "
        "FOREIGN KEY (sender_id) REFERENCES users (id) ON DELETE SET NULL"
        ");";

    if (execute_sql(create_users) != 0) return -1;
    if (execute_sql(create_chats) != 0) return -1;
    if (execute_sql(create_chat_members) != 0) return -1;
    if (execute_sql(create_messages) != 0) return -1;
    if (execute_sql("PRAGMA journal_mode=WAL;") != 0) return -1;

    return 0;
}
