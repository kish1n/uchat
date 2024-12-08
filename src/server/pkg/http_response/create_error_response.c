#include "response.h"

/**
 * Creates a JSON error response string with the given error message and status code.
 */
char* create_error_response(const char *error_msg, STATUS_CODE status_code) {
    return create_response(error_msg, status_code);
}
