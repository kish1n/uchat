#include "worker.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Функция, выполняемая каждым потоком
void* worker_thread(void *arg) {
    Worker *worker = (Worker *)arg;

    while (worker->is_running) {
        if (worker->task) {
            worker->task(worker->service); // Выполнение задачи
        }
        sleep(1); // Задержка для имитации работы
    }

    return NULL;
}

// Создание и инициализация воркера
Worker* worker_create(Service *service, int thread_count, void (*task)(Service *service)) {
    Worker *worker = (Worker *)malloc(sizeof(Worker));
    if (!worker) {
        fprintf(stderr, "Error: could not allocate memory for Worker\n");
        return NULL;
    }

    worker->service = service;
    worker->thread_count = thread_count;
    worker->is_running = 1;
    worker->task = task;
    worker->threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
    if (!worker->threads) {
        fprintf(stderr, "Error: could not allocate memory for threads\n");
        free(worker);
        return NULL;
    }

    return worker;
}

// Запуск воркера
void worker_start(Worker *worker) {
    if (!worker || !worker->threads || worker->thread_count <= 0) {
        fprintf(stderr, "Error: worker not properly initialized\n");
        return;
    }

    for (int i = 0; i < worker->thread_count; i++) {
        if (pthread_create(&worker->threads[i], NULL, worker_thread, (void *)worker) != 0) {
            fprintf(stderr, "Error: could not create thread %d\n", i);
        }
    }

    printf("Worker started with %d threads\n", worker->thread_count);
}

// Остановка воркера
void worker_stop(Worker *worker) {
    if (!worker) return;

    worker->is_running = 0;

    for (int i = 0; i < worker->thread_count; i++) {
        if (pthread_join(worker->threads[i], NULL) != 0) {
            fprintf(stderr, "Error: could not join thread %d\n", i);
        }
    }

    printf("Worker stopped\n");
}

// Освобождение ресурсов воркера
void worker_free(Worker *worker) {
    if (!worker) return;

    free(worker->threads);
    free(worker);
}
