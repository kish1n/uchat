#include "service.h"
#include <libwebsockets.h>
#include <stdlib.h>
#include <json-c/json.h>
#include "../services/service.h"
#include "../pkg/httputils/httputils.h"
#include "../pkg/http_response/response.h"
#include "../pkg/ws/ws.h"
#include "../services/messenger/messenger.h"
#include "../db/core/core.h"
#include "auth/auth_handlers.h"

enum MHD_Result router(void *cls,
                       struct MHD_Connection *connection,
                       const char *url,
                       const char *method,
                       const char *version,
                       const char *upload_data,
                       size_t *upload_data_size,
                       void **con_cls) {
    Config config;
    if (load_config("config.yaml", &config) != 0) {
        logging(ERROR, "Failed to load config");

        return EXIT_FAILURE;
    }

    if (!db) {
        if (init_db("uchat.db") != SQLITE_OK) {
            logging(ERROR, "Failed to initialize database");
            return EXIT_FAILURE;
        }
        db = get_db();
    }

    HttpContext context = {
        .cls = cls,
        .connection = connection,
        .url = url,
        .method = method,
        .version = version,
        .upload_data = upload_data,
        .upload_data_size = upload_data_size,
        .con_cls = con_cls,
        .db_conn = db
    };

    if (starts_with(url, "/auth/")) {
        const char *sub_url = url + strlen("/auth");

        if (strcmp(sub_url, "/register") == 0 && strcmp(method, "POST") == 0) {
            //req:  {"username": "username","password": "password"}
            //resp: { "status": status_code, "message": "details" }
            return handle_register(&context);
        }
        if (strcmp(sub_url, "/login") == 0 && strcmp(method, "POST") == 0) {
            //req:  {"username": "username","password": "password"}
            //resp: { "status": status_code, "message": "details" }
            return handle_login(&context);
        }
        if (strcmp(sub_url, "/logout") == 0 && strcmp(method, "GET") == 0) {
            //resp: { "status": status_code, "message": "details" }
            return handle_logout(&context);
        }
        if (strcmp(sub_url, "/update_username") == 0 && strcmp(method, "PATCH") == 0) {
            //req:  {"new_username": "new_username", "password": "password"}
            //resp: { "status": status_code, "message": "details" }
            return handle_update_username(&context);
        }

    } else if (starts_with(url, "/messages/")) {
        const char *sub_url = url + strlen("/messages");

        if (strcmp(sub_url, "/send") == 0 && strcmp(method, "POST") == 0) {
            //req:  {"chat_id": "chat_id", "content": "message"}
            //resp: { "status": status_code, "message": "details" }
            return handle_send_message(&context);
        }

        //TODO
        // if (strcmp(sub_url, "/delete") == 0 && strcmp(method, "DELETE") == 0) {
        //     return handle_delete_message(&context); //{"message_id": "message_id"}
        // }
        //TODO
        // if (strcmp(sub_url, "/edit") == 0 && strcmp(method, "PATCH") == 0) {
        //     return handle_edit_message(&context); //{"message_id": "message_id", "content": "new_content"}
        // }

        const char *id_str = sub_url + strlen("/history/");
        int chat_id = atoi(id_str);
        if (chat_id > 0) {
            context.url = id_str;
            return handle_get_chat_history(&context);
            //meth: GET
            //link: "/messages/history/{chat_id}"
            //success-resp:
            //[
            //    { "id": "message_id", "sender": "username", "message": "message_text", "sent_at": "time format datetime" },
            //    { "id": "message_id", "sender": "username", "message": "message_text", "sent_at": " 2024-12-08 16:50:18" },
            //    ...
            //]
            //bad:  {"status": status_code, "message": "details"}
        }

        return prepare_simple_response("Invalid or missing 'chat_id'", STATUS_BAD_REQUEST, NULL, &context);

    } else if (starts_with(url, "/chats/")) {
        const char *sub_url = url + strlen("/chats");

        if (strcmp(sub_url, "/create_private") == 0 && strcmp(method, "POST") == 0) {
            //req:  {"with_user": "username"}
            //resp: { "status": status_code, "message": "details" }
            return handle_create_private_chat(&context);
        }
        if (strcmp(sub_url, "/create_group") == 0 && strcmp(method, "POST") == 0) {
            //req:  {"name": "chat_name", "users": ["username1", "username2", ...]}
            //resp: { "status": status_code, "message": "details" }
            return handle_create_group_chat(&context);
        }
        if (strcmp(sub_url, "/update_name") == 0 && strcmp(method, "PATCH") == 0) {
            //req:  {"chat_id": "chat_id", "name": "new_name"}
            //resp: { "status": status_code, "message": "details" }
            return handle_update_chat_name(&context);
        }
        if (strcmp(sub_url, "/add_member") == 0 && strcmp(method, "POST") == 0) {
            //req:  {"chat_id": "chat_id", "username": "username"}
            //resp: { "status": status_code, "message": "details" }
            return handle_add_member_to_chat(&context);
        }
        if (strcmp(sub_url, "/remove_member") == 0 && strcmp(method, "DELETE") == 0) {
            //req:  {"chat_id": "chat_id", "username": "username"}
            //resp: { "status": status_code, "message": "details" }
            return handle_remove_member_from_chat(&context);
        }
        if (strcmp(sub_url, "/leave") == 0 && strcmp(method, "DELETE") == 0) {
            //req:  {"chat_id": "chat_id"}
            //resp: { "status": status_code, "message": "details" }
            return handle_leave_chat(&context);
        }
        if (strcmp(sub_url, "/delete") == 0 && strcmp(method, "DELETE") == 0) {
            //req:  {"chat_id": "chat_id"}
            //resp: { "status": status_code, "message": "details" }
            return handle_delete_chat(&context);
        }

        if (starts_with(sub_url, "/info/") && strcmp(method, "GET") == 0) {
            const char *id_str = sub_url + strlen("/info/");
            int chat_id = atoi(id_str);
            if (chat_id > 0) {
                context.url = id_str;
                //meth: GET
                //link: "/chats/info/{chat_id}"
                //success-resp:
                //{
                //  "chat_name": "name", "members": [
                //      { "username": "username", "is_admin": false },
                //      { "username": "username", "is_admin": false },
                //  ]
                //}
                //bad:  {"status": status_code, "message": "details"}
                return handle_get_chat_info(&context);
            }

            return prepare_simple_response("Invalid or missing 'chat_id'", STATUS_BAD_REQUEST, NULL, &context);

        }

    } else if (starts_with(url, "/user/")) {
        const char *sub_url = url + strlen("/user");

        if (strcmp(sub_url, "/chats") == 0 && strcmp(method, "GET") == 0) {
            //resp-success: [
            //  { "id": 1, "name": "syka", "last_message_id": 4 },
            //  { "id": 2, "name": "private_first_second", "last_message_id": null }
            //]
            //IF LAST MESSAGE ID IS NULL, THEN THERE IS NO MESSAGES IN CHAT
            //resp-bad: {"status": status_code, "message": "details"}

            return handle_get_user_chats(&context);
        }
    }

    return prepare_simple_response("Endpoint not found", STATUS_NOT_FOUND, NULL, &context);
}
