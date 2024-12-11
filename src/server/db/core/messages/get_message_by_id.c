#include "messages.h"
#include "../../../pkg/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sodium.h> // Для работы с шифрованием/дешифрованием

int get_message_by_id(sqlite3 *db, int message_id, Message *message, Config *cfg) {
    if (!cfg) {
        logging(ERROR, "Config is NULL");
        return -1;
    }

    const char *query = "SELECT id, chat_id, sender_id, content, sent_at FROM messages WHERE id = ?";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare statement: %s", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, message_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        message->id = sqlite3_column_int(stmt, 0);
        message->chat_id = sqlite3_column_int(stmt, 1);
        strncpy(message->sender_id, (const char *)sqlite3_column_text(stmt, 2), sizeof(message->sender_id) - 1);

        // Извлечение зашифрованного сообщения
        const unsigned char *encrypted_message = (const unsigned char *)sqlite3_column_blob(stmt, 3);
        size_t encrypted_length = sqlite3_column_bytes(stmt, 3);

        if (encrypted_message && encrypted_length > crypto_secretbox_NONCEBYTES) {
            // Разделяем nonce и зашифрованный текст
            const unsigned char *nonce = encrypted_message;
            const unsigned char *ciphertext = encrypted_message + crypto_secretbox_NONCEBYTES;
            size_t ciphertext_length = encrypted_length - crypto_secretbox_NONCEBYTES;

            unsigned char decrypted_message[sizeof(message->content)];
            memset(decrypted_message, 0, sizeof(decrypted_message));

            // Дешифровка сообщения
            if (crypto_secretbox_open_easy(decrypted_message, ciphertext, ciphertext_length, nonce,
                                           (unsigned char *)cfg->security.message_secret) == 0) {
                strncpy(message->content, (const char *)decrypted_message, sizeof(message->content) - 1);
            } else {
                logging(ERROR, "Failed to decrypt message with ID %d", message_id);
                strncpy(message->content, "Failed to decrypt message", sizeof(message->content) - 1);
            }
        } else {
            logging(ERROR, "Invalid encrypted message length for message ID %d", message_id);
            strncpy(message->content, "Invalid encrypted message", sizeof(message->content) - 1);
        }

        strncpy(message->sent_at, (const char *)sqlite3_column_text(stmt, 4), sizeof(message->sent_at) - 1);
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}
