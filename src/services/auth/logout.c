#include "auth_handlers.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../pkg/config/config.h"

int handle_logout(HttpContext *context) {
    if (!context) {
        logging(ERROR, "Invalid context in handle_logout");
        return MHD_NO;
    }

    // Validate JWT
    char *user_id = NULL;
    int auth_status = validate_auth_token(context, &user_id);
    if (auth_status != MHD_HTTP_OK) {
        const char *error_msg = "{\"status\":\"error\",\"message\":\"Unauthorized\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(context->connection, auth_status, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Logout logic (e.g., invalidate the token in a token blacklist, if implemented)
    // Note: JWTs are stateless, so you can't "destroy" them without server-side state.
    logging(INFO, "User %s logged out", user_id);

    free(user_id);

    // Respond with success
    const char *success_msg = "{\"status\":\"success\",\"message\":\"Logged out successfully\"}";
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(success_msg), (void *)success_msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(context->connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}
