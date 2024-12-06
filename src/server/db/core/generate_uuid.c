#include <uuid/uuid.h>
#include <stdio.h>

void generate_uuid(char *uuid_str) {
    uuid_t binuuid;
    uuid_generate_random(binuuid);
    uuid_unparse_lower(binuuid, uuid_str);
}
