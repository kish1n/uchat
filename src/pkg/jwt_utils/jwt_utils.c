#include "jwt_utils.h"
#include <jwt.h>
#include <stdlib.h>
#include <string.h>
#include "../config/config.h"

char *generate_jwt(const char *user_id, const char *secret_key, int expiration_time) {
    jwt_t *jwt = NULL;
    char *token = NULL;

    if (jwt_new(&jwt) != 0) {
        logging(ERROR, "Failed to create JWT object");
        return NULL;
    }

    // Add claims to the JWT
    jwt_add_grant(jwt, "sub", user_id); // "sub" is the user_id
    jwt_add_grant_int(jwt, "exp", time(NULL) + expiration_time);

    // Encode the JWT
    token = jwt_encode_str(jwt); // Now using the correct function
    if (!token) {
        logging(ERROR, "Failed to encode JWT");
        jwt_free(jwt);
        return NULL;
    }

    jwt_free(jwt);
    return token; // The caller is responsible for freeing this string
}


int verify_jwt(const char *token, const char *secret_key, char **decoded_user_id) {
    jwt_t *jwt = NULL;

    if (jwt_decode(&jwt, token, (unsigned char *)secret_key, strlen(secret_key)) != 0) {
        logging(ERROR, "Failed to decode JWT token");
        return -1;
    }

    const char *user_id = jwt_get_grant(jwt, "sub");
    if (user_id) {
        *decoded_user_id = strdup(user_id); // Dynamically allocate memory for user_id
        jwt_free(jwt);
        return 1; // Valid token
    }

    logging(ERROR, "Failed to extract 'sub' from JWT payload");
    jwt_free(jwt);
    return -1; // Invalid token
}
