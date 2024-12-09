#include "threads.h"

void enqueue_task(TaskQueue *queue, HttpContext *context) {
    logging(DEBUG, "Enqueueing task for URL: %s", context->url);

    Task *task = malloc(sizeof(Task));
    if (!task) {
        logging(ERROR, "Failed to allocate memory for task");
        return;
    }

    task->context = context;  // Копируем контекст задачи
    task->next = NULL;

    pthread_mutex_lock(&queue->mutex);
    if (queue->rear) {
        queue->rear->next = task;
    } else {
        queue->front = task;
    }
    queue->rear = task;

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);

    logging(DEBUG, "Task enqueued successfully");
}
