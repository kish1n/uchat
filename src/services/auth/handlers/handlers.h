#ifndef HANDLERS_H
#define HANDLERS_H

#include <microhttpd.h>
#include <string.h>

int login_handler(struct MHD_Connection *connection);
int register_handler(struct MHD_Connection *connection);

#endif