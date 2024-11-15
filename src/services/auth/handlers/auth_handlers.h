#ifndef AUTH_HANDLERS_H
#define AUTH_HANDLERS_H

#include <microhttpd.h>
#include "../auth.h"

// Обработчик для регистрации пользователя
int handle_register_request(Auth *auth, struct MHD_Connection *connection);

#endif // AUTH_HANDLERS_H
