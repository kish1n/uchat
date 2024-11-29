#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdlib.h>

/**
 * Enum for HTTP status codes
 */
typedef enum {
    STATUS_OK = 200,
    STATUS_CREATED = 201,
    STATUS_ACCEPTED = 202,
    STATUS_FOUND = 302,
    STATUS_BAD_REQUEST = 400,
    STATUS_UNAUTHORIZED = 401,
    STATUS_FORBIDDEN = 403,
    STATUS_NOT_FOUND = 404,
    STATUS_CONFLICT = 409,
    STATUS_INTERNAL_SERVER_ERROR = 500,
} STATUS_CODE;

/**
 * Creates an HTTP response in JSON format.
 *
 * @param msg The message body of the response.
 * @param status_code The HTTP status code (e.g., 200, 201).
 * @return A dynamically allocated string containing the response. Caller must free it.
 */
char* create_response(const char *msg, STATUS_CODE status_code);

/**
 * Helper function to generate a response for errors.
 *
 * @param error_msg The error message to include in the response.
 * @param status_code The HTTP status code for the error.
 * @return A dynamically allocated string containing the error response. Caller must free it.
 */
char* create_error_response(const char *error_msg, STATUS_CODE status_code);

#endif // RESPONSE_H
