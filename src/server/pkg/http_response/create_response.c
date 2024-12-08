#include "response.h"

/**
 * Creates a JSON response string with the given message and status code.
 */
char* create_response(const char *msg, STATUS_CODE status_code) {
    const char *template = "{ \"status\": %d, \"message\": \"%s\" }";
    size_t response_size = snprintf(NULL, 0, template, status_code, msg) + 1;

    char *response = malloc(response_size);
    if (!response) {
        fprintf(stderr, "Memory allocation failed for response\n");
        return NULL;
    }

    snprintf(response, response_size, template, status_code, msg);
    return response;
}
