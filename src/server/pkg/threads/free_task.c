#include "threads.h"

void free_task(Task *task) {
    if (task->context->url) free((char *)task->context->url);
    if (task->context->method) free((char *)task->context->method);
    if (task->context->upload_data) free((char *)task->context->upload_data);
    free(task);
}
