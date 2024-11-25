#include "jwt_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

char* generate_jwt(const char *user_id, const char *secret_key, int expiration_time) {
    jwt_t *jwt = NULL;
    time_t now = time(NULL);
    char *token = NULL;

    // Создаём JWT объект
    if (jwt_new(&jwt) != 0) {
        fprintf(stderr, "Failed to create JWT object\n");
        return NULL;
    }

    // Добавляем стандартные claims
    jwt_add_grant(jwt, "sub", user_id); // Указываем, что токен принадлежит user_id
    jwt_add_grant_int(jwt, "exp", now + expiration_time); // Устанавливаем время жизни токена

    // Устанавливаем секретный ключ
    if (jwt_set_alg(jwt, JWT_ALG_HS256, (unsigned char *)secret_key, strlen(secret_key)) != 0) {
        fprintf(stderr, "Failed to set JWT algorithm and key\n");
        jwt_free(jwt);
        return NULL;
    }

    // Генерируем токен в строку
    token = jwt_encode_str(jwt);
    jwt_free(jwt);

    if (!token) {
        fprintf(stderr, "Failed to encode JWT token\n");
        return NULL;
    }

    return token;
}

int verify_jwt(const char *token, const char *secret_key, char **decoded_user_id) {
    jwt_t *jwt = NULL;
    int ret = 0;

    if (jwt_decode(&jwt, token, (unsigned char *)secret_key, strlen(secret_key)) != 0) {
        fprintf(stderr, "Failed to decode JWT token\n");
        return -1;
    }

    const char *user_id = jwt_get_grant(jwt, "sub");
    if (user_id) {
        *decoded_user_id = strdup(user_id);
        ret = 1; // Успешно
    } else {
        fprintf(stderr, "Failed to extract 'sub' from JWT\n");
        ret = -1;
    }

    jwt_free(jwt);
    return ret;
}
