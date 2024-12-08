#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <json-c/json_object.h>
#include "../httputils/httpcontext.h"
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

int prepare_simple_response(const char *error_msg, const STATUS_CODE status_code, json_object *json_data, HttpContext *context);

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
int prepare_response(const STATUS_CODE status_code,  json_object *json_data, HttpContext *context);
#endif // RESPONSE_H
