#include <stdio.h>
#include "src/services/auth/auth.h"

int main() {

    Auth *auth_service = auth_create("../config.yaml", "secret");
    if (!auth_service) {
        fprintf(stderr, "Failed to initialize auth service\n");
        return 1;
    }

    auth_start(auth_service);

    printf("Auth service is running. Press Enter to stop...\n");
    getchar();

    auth_stop(auth_service);
    auth_free(auth_service);

    return 0;
}
