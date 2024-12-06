-- Drop messages table and related indexes
DROP INDEX IF EXISTS idx_messages_chat_id;
DROP INDEX IF EXISTS idx_messages_sender_id;
DROP INDEX IF EXISTS idx_messages_sent_at;
DROP TABLE IF EXISTS messages;

-- Drop chat_members table and related indexes
DROP INDEX IF EXISTS idx_chat_members_chat_id;
DROP INDEX IF EXISTS idx_chat_members_user_id;
DROP TABLE IF EXISTS chat_members;

-- Drop chats table and related indexes
DROP TABLE IF EXISTS "users" CASCADE;
DROP TABLE IF EXISTS "messages" CASCADE;
DROP TABLE IF EXISTS "chats" CASCADE;
DROP TABLE IF EXISTS "chat_members" CASCADE;
DROP EXTENSION pgcrypto CASCADE;

-- Drop pgcrypto extension (if you don't need it anymore)
DROP EXTENSION IF EXISTS pgcrypto;
