#include "ws.h"

int ws_chat_handler(struct lws *wsi, enum lws_callback_reasons reason,
                    void *user, void *in, size_t len) {
    static char chat_id[64] = ""; // Переменная для хранения chat_id

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: {
            // Извлекаем URL из заголовка
            char url[256];
            if (lws_hdr_copy(wsi, url, sizeof(url), WSI_TOKEN_GET_URI) > 0) {
                // Ожидаем, что URL имеет формат /chat/{chat_id}
                const char *chat_prefix = "/chat/";
                if (strncmp(url, chat_prefix, strlen(chat_prefix)) == 0) {
                    strncpy(chat_id, url + strlen(chat_prefix), sizeof(chat_id) - 1);
                    chat_id[sizeof(chat_id) - 1] = '\0'; // Гарантируем завершение строки
                    logging(INFO, "Chat ID extracted: %s", chat_id);
                } else {
                    logging(ERROR, "Invalid WebSocket URL: %s", url);
                    return -1; // Закрываем соединение
                }
            } else {
                logging(ERROR, "Failed to extract URL");
                return -1; // Закрываем соединение
            }

            // Добавляем клиента
            add_client(chat_id, "unknown_user", wsi); // Здесь user_id можно позже заменить
            break;
        }

        case LWS_CALLBACK_RECEIVE: {
            logging(INFO, "Message received in chat_id=%s: %.*s", chat_id, (int)len, (char *)in);
            broadcast_message(chat_id, (const char *)in);
            break;
        }

        case LWS_CALLBACK_CLOSED: {
            logging(INFO, "WebSocket connection closed");
            remove_client(wsi);
            break;
        }

        default:
            logging(INFO, "Unhandled WebSocket callback reason: %d", reason);
        break;
    }
    return 0;
}
