#include "httputils.h"
#include <string.h>

// Checks if the given URL starts with the specified prefix
int starts_with(const char *url, const char *prefix) {
    return strncmp(url, prefix, strlen(prefix)) == 0;
}
