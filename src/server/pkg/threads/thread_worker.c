#include "threads.h"

void *thread_worker(void *arg) {
    logging(DEBUG, "Thread %lu started", pthread_self());
    TaskQueue *queue = (TaskQueue *)arg;

    while (1) {
        Task *task = dequeue_task(queue);
        if (!task) {
            logging(DEBUG, "Thread %lu exiting due to stop signal", pthread_self());
            break;
        }
        logging(DEBUG, "Thread %lu is processing task for URL: %s", pthread_self(), task->context->url);

        // Обработка задачи
        router(task->context);

        free_task(task);
    }

    return NULL;
}
