#ifndef JWT_UTILS_H
#define JWT_UTILS_H

#include "../config/config.h"
#include <stdbool.h>

char *generate_jwt(const char *user_id, const char *secret_key, int expiration_time);
int verify_jwt(const char *token, const char *secret_key, char **decoded_user_id);
const char *extract_jwt_from_authorization_header(struct MHD_Connection *connection);
int validate_auth_token(HttpContext *context, char **user_id);

#endif // JWT_UTILS_H
