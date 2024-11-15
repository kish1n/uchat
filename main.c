#include <stdio.h>
#include <unistd.h>
#include "src/services/service.h"
#include "src/pkg/worker/worker.h"

int main() {
    // Инициализация первого сервиса
    Service *service1 = service_create("../config.yaml");
    if (!service1) {
        fprintf(stderr, "Failed to initialize service 1\n");
        return 1;
    }

    // Инициализация второго сервиса
    Service *service2 = service_create("../config.yaml");
    if (!service2) {
        fprintf(stderr, "Failed to initialize service 2\n");
        service_stop(service1);
        return 1;
    }

    // Создание воркеров для каждого сервиса
    Worker *worker1 = worker_create(service1, 2);  // Воркеры с 2 потоками
    if (!worker1) {
        fprintf(stderr, "Failed to create worker for service 1\n");
        service_stop(service1);
        service_stop(service2);
        return 1;
    }

    Worker *worker2 = worker_create(service2, 2);
    if (!worker2) {
        fprintf(stderr, "Failed to create worker for service 2\n");
        worker_free(worker1);
        service_stop(service1);
        service_stop(service2);
        return 1;
    }

    // Запуск воркеров
    worker_start(worker1);
    worker_start(worker2);

    // Имитация выполнения основного процесса
    printf("Services are running...\n");
    sleep(5);  // Подождите 5 секунд, чтобы дать воркерам поработать

    // Остановка воркеров и освобождение ресурсов
    worker_stop(worker1);
    worker_stop(worker2);
    worker_free(worker1);
    worker_free(worker2);

    // Остановка сервисов
    service_stop(service1);
    service_stop(service2);

    printf("Services stopped.\n");
    return 0;
}
