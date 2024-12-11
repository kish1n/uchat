#include "messages.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>

int create_message(sqlite3 *db, int chat_id, const char *sender_id, const char *content, Config *cfg) {
    const char *query = "INSERT INTO messages (chat_id, sender_id, content, sent_at) "
                        "VALUES (?, ?, ?, datetime('now'))";

    // Проверка длины ключа
    if (strlen(cfg->security.message_secret) != crypto_secretbox_KEYBYTES) {
        logging(ERROR, "Invalid message_secret %s length. Expected %d bytes", crypto_secretbox_KEYBYTES);
        return -1;
    }

    // Шифрование сообщения
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char encrypted_message[crypto_secretbox_MACBYTES + strlen(content)];
    size_t encrypted_length = sizeof(encrypted_message);

    randombytes_buf(nonce, sizeof(nonce));

    if (crypto_secretbox_easy(encrypted_message, (unsigned char *)content, strlen(content), nonce,
                              (unsigned char *)cfg->security.message_secret) != 0) {
        logging(ERROR, "Failed to encrypt message");
        return -1;
    }

    // Склеиваем nonce и зашифрованное сообщение
    size_t total_length = sizeof(nonce) + encrypted_length;
    unsigned char *final_encrypted_message = malloc(total_length);
    if (!final_encrypted_message) {
        logging(ERROR, "Memory allocation failed for final_encrypted_message");
        return -1;
    }
    memcpy(final_encrypted_message, nonce, sizeof(nonce));
    memcpy(final_encrypted_message + sizeof(nonce), encrypted_message, encrypted_length);

    // Запись в базу данных
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare statement: %s\nSQL: %s", sqlite3_errmsg(db), query);
        free(final_encrypted_message);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chat_id);
    sqlite3_bind_text(stmt, 2, sender_id, -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 3, final_encrypted_message, total_length, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    free(final_encrypted_message);

    if (result != SQLITE_DONE) {
        logging(ERROR, "Failed to execute statement: %s\nSQL: %s", sqlite3_errmsg(db), query);
        sqlite3_finalize(stmt);
        return -1;
    }

    int message_id = (int)sqlite3_last_insert_rowid(db); // Получаем ID созданного сообщения
    sqlite3_finalize(stmt);

    // Используем edit_last_message_id для обновления чата
    if (edit_last_message_id(db, chat_id, message_id) != 0) {
        logging(ERROR, "Failed to update last_message_id for chat_id=%d", chat_id);
        return -1;
    }

    logging(INFO, "Message created: chat_id=%d, sender_id=%s, message_id=%d", chat_id, sender_id, message_id);
    return message_id;
}
