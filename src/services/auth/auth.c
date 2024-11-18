#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <stdio.h>
#include <string.h>
#include <microhttpd.h>
#include <libpq-fe.h>

Auth* auth_create(const char *config_path, const char *jwt_secret) {
    Auth *auth = (Auth *)malloc(sizeof(Auth));
    if (!auth) {
        fprintf(stderr, "Error: could not allocate memory for Auth\n");
        return NULL;
    }

    auth->service = service_create(config_path);
    if (auth->service == NULL) {
        fprintf(stderr, "Error: failed to initialize Service for Auth\n");
        free(auth);
        return NULL;
    }
    strncpy(auth->jwt_secret, jwt_secret, sizeof(auth->jwt_secret) - 1);
    auth->jwt_secret[sizeof(auth->jwt_secret) - 1] = '\0';

    return auth;
}

void auth_start(Auth *auth) {
    if (!auth) {
        fprintf(stderr, "Error: auth is not initialized\n");
        return;
    }
    service_start(auth->service);
    printf("Auth service started\n");
}

void auth_stop(Auth *auth) {
    if (!auth) {
        return;
    }
    service_stop(auth->service);
    printf("Auth service stopped\n");
}

void auth_free(Auth *auth) {
    if (!auth) {
        return;
    }
    service_stop(auth->service);
    free(auth);
    printf("Auth service resources freed\n");
}
