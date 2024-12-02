#include "users.h"
#include <stdio.h>
#include <stdlib.h>

void free_user(User *user) {
    if (user) {
        free(user->id);
        free(user->username);
        free(user->passhash);
        free(user->created_at);
        free(user);
    }
}