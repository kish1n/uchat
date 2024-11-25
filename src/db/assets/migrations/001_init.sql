-- Enable pgcrypto extension for UUID generation
CREATE EXTENSION IF NOT EXISTS pgcrypto;

-- Create users table
CREATE TABLE IF NOT EXISTS "users" (
                                       "uuid" UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    "username" VARCHAR(255) UNIQUE NOT NULL,
    "passhash" VARCHAR(255) NOT NULL,
    "created_at" TIMESTAMP DEFAULT NOW()
    );

-- Add index for faster search by username
CREATE INDEX IF NOT EXISTS idx_users_username ON "users" (username);

-- Create chats table
CREATE TABLE IF NOT EXISTS chats (
                                     "id" SERIAL PRIMARY KEY,
                                     "name" VARCHAR(255),
    "is_group" BOOLEAN NOT NULL DEFAULT FALSE,
    "created_at" TIMESTAMP DEFAULT NOW()
    );

-- Add index for faster search by name
CREATE INDEX IF NOT EXISTS idx_chats_name ON chats (name);

-- Create chat_members table
CREATE TABLE IF NOT EXISTS chat_members (
                                            "chat_id" INT NOT NULL REFERENCES chats(id) ON DELETE CASCADE,
    "user_id" UUID NOT NULL REFERENCES users(uuid) ON DELETE CASCADE,
    "is_admin" BOOLEAN NOT NULL DEFAULT FALSE,
    "joined_at" TIMESTAMP DEFAULT NOW(),
    PRIMARY KEY (chat_id, user_id)
    );

-- Add index for faster search in chat_members
CREATE INDEX IF NOT EXISTS idx_chat_members_chat_id ON chat_members (chat_id);
CREATE INDEX IF NOT EXISTS idx_chat_members_user_id ON chat_members (user_id);

-- Create messages table
CREATE TABLE IF NOT EXISTS messages (
                                        "id" SERIAL PRIMARY KEY,
                                        "chat_id" INT NOT NULL REFERENCES chats(id) ON DELETE CASCADE,
    "sender_id" UUID NOT NULL REFERENCES users(uuid) ON DELETE SET NULL,
    "content" TEXT NOT NULL,
    "sent_at" TIMESTAMP DEFAULT NOW()
    );

-- Add index for faster retrieval of messages by chat
CREATE INDEX IF NOT EXISTS idx_messages_chat_id ON messages (chat_id);
CREATE INDEX IF NOT EXISTS idx_messages_sender_id ON messages (sender_id);
CREATE INDEX IF NOT EXISTS idx_messages_sent_at ON messages (sent_at);
