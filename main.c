#include <stdio.h>
#include "./src/services/service.h" // Предполагаем, что вся реализация сервиса вынесена в service.h и service.c

// Точка входа
int main() {
    Service *service = service_create("my_secret_key");
    if (!service) {
        fprintf(stderr, "Failed to create service\n");
        return 1;
    }

    service_init(service);

    // Регистрируем эндпоинты
    service_register_endpoint(service, "/login", "POST", login_handler);
    service_register_endpoint(service, "/register", "POST", register_handler);

    // Запуск сервиса
    service_start(service);

    // Уничтожение сервиса
    service_destroy(service);
    return 0;
}