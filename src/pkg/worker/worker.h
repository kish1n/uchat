#ifndef WORKER_H
#define WORKER_H

#include "../../services/service.h"
#include <pthread.h>

typedef struct {
    Service *service;
    pthread_t *threads;
    int thread_count;
    int is_running;
} Worker;

Worker* worker_create(Service *service, int thread_count);
void worker_start(Worker *worker);
void worker_stop(Worker *worker);
void worker_free(Worker *worker);

#endif // WORKER_H
