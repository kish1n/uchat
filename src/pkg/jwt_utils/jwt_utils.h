#ifndef JWT_UTILS_H
#define JWT_UTILS_H

#include <jwt.h>

// Генерация JWT токена
char* generate_jwt(const char *user_id, const char *secret_key, int expiration_time);

// Проверка JWT токена
int verify_jwt(const char *token, const char *secret_key, char **decoded_user_id);

#endif // JWT_UTILS_H
