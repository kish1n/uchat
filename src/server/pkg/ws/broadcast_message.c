#include "ws.h"

void broadcast_message(const char *chat_id, const char *message) {
    if (client_count == 0) {
        logging(WARN, "No clients connected. Message not broadcasted.");
        return;
    }

    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].chat_id, chat_id) == 0) {
            size_t len = strlen(message);
            unsigned char buffer[LWS_PRE + len];
            memcpy(&buffer[LWS_PRE], message, len);

            int bytes_written = lws_write(clients[i].wsi, &buffer[LWS_PRE], len, LWS_WRITE_TEXT);
            if (bytes_written < (int)len) {
                logging(WARN, "Failed to send complete message to user_id=%s in chat_id=%s", clients[i].user_id, chat_id);
            } else {
                logging(INFO, "Message sent to user_id=%s in chat_id=%s", clients[i].user_id, chat_id);
            }
        }
    }

    logging(INFO, "Message broadcasted to chat_id=%s: %s", chat_id, message);
}
