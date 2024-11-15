#ifndef AUTH_H
#define AUTH_H

#include "../service.h"

typedef struct {
    Service *service;
    char jwt_secret[256];
} Auth;

Auth* auth_create(const char *config_path, const char *jwt_secret);

void auth_start(Auth *auth);
void auth_stop(Auth *auth);
void auth_free(Auth *auth);

#endif // AUTH_H
