#include "ws.h"

void broadcast_message(const char *chat_id, const char *message) {
    logging(INFO, "Broadcasting message to chat_id=%s: %.*s", chat_id, message);

    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].chat_id, chat_id) == 0) {
            size_t message_len = strlen(message);

            if (message_len > MAX_MESSAGE_SIZE) {
                logging(WARN, "Message too long, skipping broadcast for chat_id=%s", chat_id);
                continue;
            }

            unsigned char buffer[LWS_PRE + MAX_MESSAGE_SIZE];
            memset(buffer, 0, sizeof(buffer));
            memcpy(&buffer[LWS_PRE], message, message_len);

            int bytes_written = lws_write(clients[i].wsi, &buffer[LWS_PRE], message_len, LWS_WRITE_TEXT);

            if (bytes_written < (int)message_len) {
                logging(WARN, "Failed to send full message to user_id=%s in chat_id=%s", clients[i].user_id, chat_id);
            } else {
                logging(INFO, "Message sent to user_id=%s in chat_id=%s", clients[i].user_id, chat_id);
            }
        }
    }
}

