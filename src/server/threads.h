#ifndef THREADS_H
#define THREADS_H
#include <pthread.h>
#include "pkg/httputils/httpcontext.h"
#include <stdbool.h>

#define THREAD_POOL_SIZE 4

typedef struct Task {
    HttpContext context;
    struct Task *next;
} Task;

typedef struct {
    Task *front;
    Task *rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool stop;
} TaskQueue;

typedef struct {
    pthread_t *threads;
    size_t thread_count;
    TaskQueue task_queue;
} ThreadPool;

void init_task_queue(TaskQueue *queue);
void enqueue_task(TaskQueue *queue, HttpContext *context);
Task *dequeue_task(TaskQueue *queue);
void *thread_worker(void *arg);
ThreadPool *thread_pool_init(size_t thread_count);
void thread_pool_destroy(ThreadPool *pool);


void test_enqueue(TaskQueue *queue);

#endif