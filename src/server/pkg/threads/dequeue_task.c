#include "threads.h"

Task *dequeue_task(TaskQueue *queue) {
    pthread_mutex_lock(&queue->mutex);

    while (!queue->front && !queue->stop) {
        logging(DEBUG, "Waiting for tasks in thread %lu", pthread_self());
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
    logging(DEBUG, "Dequeued task for URL: %s", task->context->url);
    return task;
}
