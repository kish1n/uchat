#ifndef WORKER_H
#define WORKER_H

#include "../../services/service.h"
#include <pthread.h>

typedef struct {
    Service *service;                      // Указатель на сервис
    pthread_t *threads;                    // Массив потоков
    int thread_count;                      // Количество потоков
    int is_running;                        // Флаг состояния воркера
    void (*task)(Service *service);        // Указатель на функцию задачи
} Worker;

// Создание и инициализация воркера
Worker* worker_create(Service *service, int thread_count, void (*task)(Service *service));

// Запуск воркера
void worker_start(Worker *worker);

// Остановка воркера
void worker_stop(Worker *worker);

// Освобождение ресурсов воркера
void worker_free(Worker *worker);

#endif // WORKER_H
