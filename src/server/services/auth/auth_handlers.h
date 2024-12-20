#ifndef AUTH_HANDLERS_H
#define AUTH_HANDLERS_H

#include <microhttpd.h>
#include "../service.h"

int handle_register(HttpContext *context);
int handle_login(HttpContext *context);
int handle_logout(HttpContext *context);
int handle_update_username(HttpContext *context);

#endif // AUTH_HANDLERS_H
