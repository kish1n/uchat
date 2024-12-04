#ifndef MESSAGES_H
#define MESSAGES_H

#include <microhttpd.h>
#include <libpq-fe.h>
#include "../service.h"

int handle_send_message(HttpContext *context);
int handle_edit_message(HttpContext *context);
int handle_delete_message(HttpContext *context);

int handle_delete_chat(HttpContext *context);
int handle_create_group_chat(HttpContext *context);
int handle_create_private_chat(HttpContext *context);
int handle_add_member_to_chat(HttpContext *context);
int handle_remove_member_from_chat(HttpContext *context);
int handle_leave_chat(HttpContext *context);
int handle_update_chat_name(HttpContext *context);

//TODO create:
//int handle_get_chat_members(HttpContext *context);
//int handle_get_chat_messages(HttpContext *context);

#endif
