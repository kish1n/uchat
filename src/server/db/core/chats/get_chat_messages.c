#include "chats.h"
#include <sqlite3.h>
#include <json-c/json.h>
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../pkg/config/config.h"

char *get_chat_messages(sqlite3 *db, int chat_id, Config *cfg) {
    if (!db || !cfg) {
        fprintf(stderr, "Invalid parameters to get_chat_messages\n");
        return NULL;
    }

    const char *query =
        "SELECT m.id AS message_id, u.username AS sender, m.content AS message, m.sent_at "
        "FROM messages m "
        "JOIN users u ON m.sender_id = u.id "
        "WHERE m.chat_id = ? "
        "ORDER BY m.sent_at ASC";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    sqlite3_bind_int(stmt, 1, chat_id);

    struct json_object *response_array = json_object_new_array();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        struct json_object *message_obj = json_object_new_object();

        const char *message_id = (const char *)sqlite3_column_text(stmt, 0);
        const char *sender = (const char *)sqlite3_column_text(stmt, 1);
        const unsigned char *encrypted_message = (const unsigned char *)sqlite3_column_blob(stmt, 2);
        size_t encrypted_length = sqlite3_column_bytes(stmt, 2);
        const char *sent_at = (const char *)sqlite3_column_text(stmt, 3);

        char decrypted_message[1024] = {0};

        // Дешифровка сообщения
        if (encrypted_message && encrypted_length > crypto_secretbox_NONCEBYTES) {
            const unsigned char *nonce = encrypted_message;
            const unsigned char *ciphertext = encrypted_message + crypto_secretbox_NONCEBYTES;
            size_t ciphertext_length = encrypted_length - crypto_secretbox_NONCEBYTES;

            if (crypto_secretbox_open_easy((unsigned char *)decrypted_message, ciphertext, ciphertext_length, nonce,
                                           (unsigned char *)cfg->security.message_secret) != 0) {
                fprintf(stderr, "Failed to decrypt message with ID %s\n", message_id);
                strncpy(decrypted_message, "Failed to decrypt", sizeof(decrypted_message) - 1);
            }
        } else {
            fprintf(stderr, "Invalid encrypted message length for message ID %s\n", message_id);
            strncpy(decrypted_message, "Invalid encrypted message", sizeof(decrypted_message) - 1);
        }

        json_object_object_add(message_obj, "id", json_object_new_string(message_id));
        json_object_object_add(message_obj, "sender", json_object_new_string(sender));
        json_object_object_add(message_obj, "message", json_object_new_string(decrypted_message));
        json_object_object_add(message_obj, "sent_at", json_object_new_string(sent_at));

        json_object_array_add(response_array, message_obj);
    }

    char *response = strdup(json_object_to_json_string(response_array));

    json_object_put(response_array);
    sqlite3_finalize(stmt);

    return response;
}
