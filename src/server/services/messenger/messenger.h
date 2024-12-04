#ifndef MESSAGES_H
#define MESSAGES_H

#include <microhttpd.h>
#include <libpq-fe.h>
#include "../service.h"

int handle_send_message(HttpContext *context);
int handle_edit_message(HttpContext *context);
int handle_delete_message(HttpContext *context);

int handle_create_chat(HttpContext *context);
int handle_delete_chat(HttpContext *context);
int handle_create_private_chat(HttpContext *context);

//TODO create:
//int handle_add_chat_member(HttpContext *context);
//int handle_remove_chat_member(HttpContext *context);
//int handle_get_chat_members(HttpContext *context);
//int handle_get_chat_messages(HttpContext *context);

#endif
