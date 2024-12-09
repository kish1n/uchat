#include "threads.h"

void thread_pool_destroy(ThreadPool *pool) {
    pthread_mutex_lock(&pool->task_queue.mutex);
    pool->task_queue.stop = true;
    pthread_cond_broadcast(&pool->task_queue.cond);
    pthread_mutex_unlock(&pool->task_queue.mutex);

    for (size_t i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    pthread_mutex_destroy(&pool->task_queue.mutex);
    pthread_cond_destroy(&pool->task_queue.cond);
    free(pool);
}

