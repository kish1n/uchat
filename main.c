#include <stdio.h>
#include "./src/services/service.h"
#include "./src/services/auth/handlers/handlers.h"

int main() {
    Service *service = service_create("my_secret_key");
    if (!service) {
        fprintf(stderr, "Failed to create service\n");
        return 1;
    }

    service_init(service);

    //registering endpoints and his handlers
    service_register_endpoint(service, "/login", "POST", login_handler);
    service_register_endpoint(service, "/register", "POST", register_handler);

    service_start(service);

    service_destroy(service);
    return 0;
}