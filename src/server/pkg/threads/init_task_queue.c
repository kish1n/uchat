#include "threads.h"

void init_task_queue(TaskQueue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->stop = false;
}
