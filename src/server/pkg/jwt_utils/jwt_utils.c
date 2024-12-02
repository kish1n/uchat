#include "jwt_utils.h"
#include <jwt.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include "../config/config.h"
#include <time.h>

char *generate_jwt(const char *user_id, const char *secret_key, int expiration_time) {
    jwt_t *jwt = NULL;
    char *token = NULL;

    if (jwt_new(&jwt) != 0) {
        fprintf(stderr, "Failed to create JWT object\n");
        return NULL;
    }

    jwt_set_alg(jwt, JWT_ALG_HS256, (unsigned char *)secret_key, strlen(secret_key));

    jwt_add_grant(jwt, "sub", user_id); // User ID
    jwt_add_grant_int(jwt, "exp", time(NULL) + expiration_time); // Expiration

    token = jwt_encode_str(jwt);
    jwt_free(jwt);

    return token;
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

int validate_auth_token(HttpContext *context, char **user_id) {
    Config cfg;
    load_config("config.yaml", &cfg);
    // Extract token from Authorization header
    const char *jwt = extract_jwt_from_authorization_header(context->connection);
    if (!jwt) {
        logging(ERROR, "Authorization header missing");
        return MHD_HTTP_UNAUTHORIZED; // Return HTTP status 401
    }

    // Verify the token
    if (verify_jwt(jwt, cfg.security.jwt_secret, user_id) != 1) {
        logging(ERROR, "Invalid or expired JWT");
        return MHD_HTTP_UNAUTHORIZED; // Return HTTP status 401
    }

    logging(INFO, "User authenticated with ID: %s", *user_id);
    return MHD_HTTP_OK; // Success
}

const char *extract_jwt_from_authorization_header(struct MHD_Connection *connection) {
    // Get the Authorization header value
    const char *auth_header = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Authorization");
    if (!auth_header) {
        return NULL; // No Authorization header
    }

    // Check if it starts with "Bearer "
    const char prefix[] = "Bearer ";
    if (strncmp(auth_header, prefix, strlen(prefix)) != 0) {
        return NULL; // Not a Bearer token
    }

    // Return the token part
    return auth_header + strlen(prefix); // Skip "Bearer " and return the token
}