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

#endif
