#ifndef AUTH_HANDLERS_H
#define AUTH_HANDLERS_H

#include <microhttpd.h>
#include <libpq-fe.h>
#include "../service.h"

int handle_register(HttpContext *context);
int handle_login(HttpContext *context);

#endif // AUTH_HANDLERS_H
