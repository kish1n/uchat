CREATE EXTENSION IF NOT EXISTS pgcrypto; -- for gen_random_uuid()

CREATE TABLE IF NOT EXISTS "users" (
    "id" UUID PRIMARY KEY NOT NULL,
    "username" VARCHAR(255) UNIQUE NOT NULL,
    "passhash" VARCHAR(255) NOT NULL,
    "created_at" VARCHAR(255) NOT NULL
);

CREATE TABLE IF NOT EXISTS chats (
    "id" SERIAL PRIMARY KEY,
    "name" VARCHAR(255),
    "is_group" BOOLEAN NOT NULL DEFAULT FALSE,
    "created_at" TIMESTAMP DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS chat_members (
    "chat_id" INT NOT NULL REFERENCES chats(id) ON DELETE CASCADE,
    "user_id" UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    "is_admin" BOOLEAN NOT NULL DEFAULT FALSE,
    "joined_at" TIMESTAMP DEFAULT NOW(),
    PRIMARY KEY (chat_id, user_id)
);

CREATE TABLE IF NOT EXISTS messages (
    "id" SERIAL PRIMARY KEY,
    "chat_id" INT NOT NULL REFERENCES chats(id) ON DELETE CASCADE,
    "sender_id" UUID NOT NULL REFERENCES users(id) ON DELETE SET NULL,
    "content" TEXT NOT NULL,
    "sent_at" TIMESTAMP DEFAULT NOW()
);