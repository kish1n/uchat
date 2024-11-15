CREATE EXTENSION IF NOT EXISTS pgcrypto; -- for gen_random_uuid()

CREATE TABLE IF NOT EXISTS "users" (
    "uuid" UUID PRIMARY KEY NOT NULL,
    "username" VARCHAR(255) UNIQUE NOT NULL,
    "passhash" VARCHAR(255) NOT NULL,
    "created_at" VARCHAR(255) NOT NULL
);

CREATE TABLE IF NOT EXISTS "messages" (
    "id" SERIAL PRIMARY KEY,
    "sender_id" UUID NOT NULL REFERENCES "users" ("uuid") ON DELETE CASCADE,
    "receiver_id" UUID REFERENCES "users" ("uuid") ON DELETE CASCADE,
    "content" TEXT NOT NULL,
    "created_at" TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    "is_deleted" BOOLEAN DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS "chat_rooms" (
    "id" SERIAL PRIMARY KEY,
    "name" VARCHAR(255) NOT NULL,
    "created_at" TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
);

CREATE TABLE IF NOT EXISTS "room_members" (
    "room_id" INT NOT NULL REFERENCES "chat_rooms" ("id") ON DELETE CASCADE,
    "user_id" UUID NOT NULL REFERENCES "users" ("uuid") ON DELETE CASCADE,
    "joined_at" TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY ("room_id", "user_id")
);