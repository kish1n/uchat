#include "messages.h"

#include "messages.h"
#include <sodium.h> // Для работы с шифрованием/дешифрованием

int edit_message(sqlite3 *db, int message_id, const char *new_content, Config *cfg) {
    if (!cfg) {
        logging(ERROR, "Config is NULL");
        return -1;
    }

    const char *query_select = "SELECT content FROM messages WHERE id = ?";
    sqlite3_stmt *stmt_select;

    // Получаем зашифрованное сообщение
    if (sqlite3_prepare_v2(db, query_select, -1, &stmt_select, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare SELECT statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt_select, 1, message_id);

    const unsigned char *encrypted_message = NULL;
    size_t encrypted_length = 0;

    if (sqlite3_step(stmt_select) == SQLITE_ROW) {
        encrypted_message = (const unsigned char *)sqlite3_column_blob(stmt_select, 0);
        encrypted_length = sqlite3_column_bytes(stmt_select, 0);
    } else {
        logging(ERROR, "Message with ID %d not found", message_id);
        sqlite3_finalize(stmt_select);
        return -1;
    }

    sqlite3_finalize(stmt_select);

    // Проверяем длину зашифрованного сообщения
    if (encrypted_length <= crypto_secretbox_NONCEBYTES) {
        logging(ERROR, "Invalid encrypted message length for message ID %d", message_id);
        return -1;
    }

    // Разделяем nonce и зашифрованный текст
    const unsigned char *nonce = encrypted_message;
    const unsigned char *ciphertext = encrypted_message + crypto_secretbox_NONCEBYTES;
    size_t ciphertext_length = encrypted_length - crypto_secretbox_NONCEBYTES;

    unsigned char decrypted_message[512];
    memset(decrypted_message, 0, sizeof(decrypted_message));

    // Дешифровка сообщения
    if (crypto_secretbox_open_easy(decrypted_message, ciphertext, ciphertext_length, nonce,
                                   (unsigned char *)cfg->security.message_secret) != 0) {
        logging(ERROR, "Failed to decrypt message ID %d", message_id);
        return -1;
    }

    logging(INFO, "Message ID %d successfully decrypted: %s", message_id, decrypted_message);

    // Обновление сообщения
    const char *query_update = "UPDATE messages SET content = ? WHERE id = ?";
    sqlite3_stmt *stmt_update;

    if (sqlite3_prepare_v2(db, query_update, -1, &stmt_update, NULL) != SQLITE_OK) {
        logging(ERROR, "Failed to prepare UPDATE statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Шифруем новое сообщение
    unsigned char nonce_new[crypto_secretbox_NONCEBYTES];
    unsigned char encrypted_new_message[512];
    unsigned long long new_content_length = strlen(new_content);

    randombytes_buf(nonce_new, sizeof(nonce_new));
    if (crypto_secretbox_easy(encrypted_new_message, (unsigned char *)new_content, new_content_length, nonce_new,
                              (unsigned char *)cfg->security.message_secret) != 0) {
        logging(ERROR, "Failed to encrypt new content for message ID %d", message_id);
        sqlite3_finalize(stmt_update);
        return -1;
    }

    // Склеиваем nonce и зашифрованное сообщение
    size_t total_length_new = sizeof(nonce_new) + new_content_length;
    unsigned char *final_encrypted_message = malloc(total_length_new);
    if (!final_encrypted_message) {
        logging(ERROR, "Memory allocation failed for encrypted message");
        sqlite3_finalize(stmt_update);
        return -1;
    }
    memcpy(final_encrypted_message, nonce_new, sizeof(nonce_new));
    memcpy(final_encrypted_message + sizeof(nonce_new), encrypted_new_message, new_content_length);

    sqlite3_bind_blob(stmt_update, 1, final_encrypted_message, total_length_new, SQLITE_STATIC);
    sqlite3_bind_int(stmt_update, 2, message_id);

    if (sqlite3_step(stmt_update) != SQLITE_DONE) {
        logging(ERROR, "Failed to execute UPDATE statement: %s\n", sqlite3_errmsg(db));
        free(final_encrypted_message);
        sqlite3_finalize(stmt_update);
        return -1;
    }

    sqlite3_finalize(stmt_update);
    free(final_encrypted_message);

    logging(INFO, "Message ID %d successfully updated with new content", message_id);
    return 0;
}
