#include "auth_handlers.h"
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "../../pkg/jwt_utils/jwt_utils.h"
#include "../../pkg/config/config.h"

// handle_logout after this endpoint frontend must delete token
int handle_logout(HttpContext *context) {
    if (!context) {
        logging(ERROR, "Invalid context in handle_logout");
        return MHD_NO;
    }

    char *user_id = NULL;
    int auth_status = validate_auth_token(context, &user_id);
    if (auth_status != MHD_HTTP_OK) {
        return prepare_response("Invalid or missing token", STATUS_UNAUTHORIZED, NULL, context);
    }

    // Logout logic (e.g., invalidate the token in a token blacklist, if implemented)
    // Note: JWTs are stateless, so you can't "destroy" them without server-side state.
    logging(INFO, "User %s logged out", user_id);

    free(user_id);

    return prepare_response("Successfully logged out", STATUS_OK, NULL, context);
}
