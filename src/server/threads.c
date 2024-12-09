#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include "pkg/httputils/httpcontext.h"
#include "services/service.h"
#include "threads.h"
#include "pkg/config/config.h"

void init_task_queue(TaskQueue *queue) {
    queue->rear = NULL;
    queue->front = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void enqueue_task(TaskQueue *queue, HttpContext *context) {
    Task *task = malloc(sizeof(Task));
    task->context = *context;
    task->next = NULL;

    pthread_mutex_lock(&queue->mutex);
    if (queue->rear) {
        queue->rear->next = task;
    }
    else {
        queue->front = task;
    }
    queue->rear = task;

    logging(DEBUG, "Task added to queue");
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

Task *dequeue_task(TaskQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (!queue->front && !queue->stop) {
        logging(DEBUG, "Waiting for tasks");
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    if (queue->stop) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    Task *task = queue->front;
    queue->front = task->next;
    if (!queue->front) {
        queue->rear = NULL;
    }

    pthread_mutex_unlock(&queue->mutex);
    return task;
}
void free_task(Task *task) {
    if (task->context.url) free(task->context.url);
    if (task->context.method) free(task->context.method);
    if (task->context.upload_data) free(task->context.upload_data);
    free(task);
}


void *thread_worker(void *arg) {
    logging(DEBUG, "Enter in function thread_worker");
    TaskQueue *queue = (TaskQueue *)arg;

    while (1)
    {
        Task *task = dequeue_task(queue);
        if(!task) {
            break;
        }
        logging(DEBUG, "Thread %lu is processing a task", pthread_self());
        /*router(
            task->context.cls,
            task->context.connection,
            task->context.url,
            task->context.method,
            task->context.version,
            task->context.upload_data,
            task->context.upload_data_size,
            task->context.con_cls
        );*/
        logging(DEBUG, "Processing request: URL=%s, Method=%s", task->context.url, task->context.method);
        handle_request(&task->context);
        free(task);
        //free_task(task);
    }
    return NULL;
    
}


/*void init_thread_pool(TaskQueue *queue) {
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&threads[i], NULL, thread_worker, queue);
        pthread_detach(threads[i]);
    }
}*/

ThreadPool *thread_pool_init(size_t thread_count) {
    logging(DEBUG, "Enter in function thread_pool_init");
    ThreadPool *pool = malloc(sizeof(ThreadPool));
    if (!pool) {
        logging(ERROR, "Failed to allocate memory for thread pool");
        return NULL;
    }

    pool->thread_count = thread_count;
    pool->threads = malloc(thread_count * sizeof(pthread_t));
    if (!pool->threads) {
        logging(ERROR, "Failed to allocate memory for threads");
        free(pool);
        return NULL;
    }

    init_task_queue(&pool->task_queue);

    for (size_t i = 0; i < thread_count; i++) {
        if(pthread_create(&pool->threads[i], NULL, thread_worker, &pool->task_queue) != 0) {
            logging(ERROR, "Failed to create thread");
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }
    return pool;
}

void thread_pool_destroy(ThreadPool *pool) {
    if (!pool) {
        return;
    }

    pthread_mutex_lock(&pool->task_queue.mutex);
    pool->task_queue.stop = true;
    pthread_cond_broadcast(&pool->task_queue.cond);
    pthread_mutex_unlock(&pool->task_queue.mutex);

    for (size_t i = 0; i < pool->thread_count; i++) {
        //pthread_cancel(pool->threads[i]);
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);

    pthread_mutex_destroy(&pool->task_queue.mutex);
    pthread_cond_destroy(&pool->task_queue.cond);

    free(pool);
    
}

void test_enqueue(TaskQueue *queue) {
    for (int i = 0; i < 5; i++) {
        HttpContext context;
        memset(&context, 0, sizeof(HttpContext));
        context.url = "/test-url";
        context.method = "GET";
        logging(DEBUG, "Adding test task %d", i + 1);
        enqueue_task(queue, &context);
        sleep(1);
    }
}