#include "service.h"  // Подключите заголовочный файл, где определена структура RequestData
#include <stdlib.h>

void free_request_data(void *con_cls) {
    RequestData *request_data = (RequestData *)con_cls;
    if (request_data) {
        if (request_data->data) free(request_data->data);
        free(request_data);
    }
}
