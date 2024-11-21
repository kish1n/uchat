#ifndef HANDLERS_H
#define HANDLERS_H

#include <microhttpd.h>
#include <string.h>
#include <libpq-fe.h>
#include <json-c/json.h>

int login_handler(struct MHD_Connection *connection);
int register_handler(struct MHD_Connection *connection, const char *upload_data, size_t *upload_data_size);
// struct json_object *get_request_json(struct MHD_Connection *connection);

#endif