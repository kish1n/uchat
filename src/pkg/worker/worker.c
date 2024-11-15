#include "worker.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* worker_task(void *arg) {
    Service *service = (Service *)arg;

    while (service->is_running) {
        log_message(INFO, "Worker task is running");
        sleep(1);
    }

    log_message(INFO, "Worker task is stopping");
    return NULL;
}

Worker* worker_create(Service *service, int thread_count) {
    Worker *worker = (Worker *)malloc(sizeof(Worker));
    if (!worker) {
        fprintf(stderr, "Error: could not allocate memory for worker\n");
        return NULL;
    }

    worker->service = service;
    worker->thread_count = thread_count;
    worker->is_running = 1;
    worker->threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
    if (!worker->threads) {
        fprintf(stderr, "Error: could not allocate memory for threads\n");
        free(worker);
        return NULL;
    }

    return worker;
}

void worker_start(Worker *worker) {
    for (int i = 0; i < worker->thread_count; i++) {
        if (pthread_create(&worker->threads[i], NULL, worker_task, (void *)worker->service) != 0) {
            fprintf(stderr, "Error: could not create thread %d\n", i);
        }
    }
    log_message(INFO, "All worker threads started");
}

void worker_stop(Worker *worker) {
    worker->is_running = 0;

    for (int i = 0; i < worker->thread_count; i++) {
        if (pthread_join(worker->threads[i], NULL) != 0) {
            fprintf(stderr, "Error: could not join thread %d\n", i);
        }
    }
    log_message(INFO, "All worker threads stopped");
}

void worker_free(Worker *worker) {
    if (worker) {
        free(worker->threads);
        free(worker);
    }
}
