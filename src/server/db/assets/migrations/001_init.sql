CREATE TABLE IF NOT EXISTS "users" (
   "id" TEXT PRIMARY KEY NOT NULL, -- UUID как TEXT
   "username" TEXT UNIQUE NOT NULL,
   "passhash" TEXT NOT NULL,
   "created_at" DATETIME DEFAULT CURRENT_TIMESTAMP -- Дата создания
);

CREATE TABLE IF NOT EXISTS "chats" (
   "id" INTEGER PRIMARY KEY AUTOINCREMENT, -- SERIAL заменён на AUTOINCREMENT
   "name" TEXT,
   "is_group" BOOLEAN NOT NULL DEFAULT 0, -- SQLite использует 0/1 для BOOL
   "created_at" DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS chat_members (
    "chat_id" INTEGER NOT NULL, -- Ссылается на "chats.id"
    "user_id" TEXT NOT NULL,    -- Ссылается на "users.id"
    "is_admin" BOOLEAN NOT NULL DEFAULT 0,
    "joined_at" DATETIME DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (chat_id, user_id),
    FOREIGN KEY (chat_id) REFERENCES chats (id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
    );

CREATE TABLE IF NOT EXISTS messages (
    "id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "chat_id" INTEGER NOT NULL, -- Ссылается на "chats.id"
    "sender_id" TEXT NOT NULL,  -- Ссылается на "users.id"
    "content" TEXT NOT NULL,
    "sent_at" DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (chat_id) REFERENCES chats (id) ON DELETE CASCADE,
    FOREIGN KEY (sender_id) REFERENCES users (id) ON DELETE SET NULL
);
