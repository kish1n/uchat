#include "threads.h"

ThreadPool *thread_pool_init(size_t thread_count) {
    ThreadPool *pool = malloc(sizeof(ThreadPool));
    pool->thread_count = thread_count;
    pool->threads = malloc(thread_count * sizeof(pthread_t));

    init_task_queue(&pool->task_queue);

    for (size_t i = 0; i < thread_count; i++) {
        pthread_create(&pool->threads[i], NULL, thread_worker, &pool->task_queue);
    }

    return pool;
}

