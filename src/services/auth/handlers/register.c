#include "handlers.h"

int register_handler(struct MHD_Connection *connection) {
    const char *response = "{\"message\": \"Register successful\"}";
    struct MHD_Response *http_response = MHD_create_response_from_buffer(strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, http_response);
    MHD_destroy_response(http_response);
    return ret;
}