#ifndef CHATS_H
#define CHATS_H

#include "../../pkg/config/config.h"
#include "../service.h"
#include <libpq-fe.h>

// Function to create a new chat
int handle_create_chat(HttpContext *context);
int handle_delete_chat(HttpContext *context)
#endif // CHATS_H
