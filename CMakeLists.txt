cmake_minimum_required(VERSION 3.29)
project(uchat_auth C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.1)

# Найти библиотеку OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${YAML_INCLUDE_DIRS} /usr/include/postgresql)
link_directories(/usr/lib/x86_64-linux-gnu)

add_executable(uchat_auth
        main.c

        src/server/db/core/db.c
        src/server/db/core/migrations.c
        src/server/db/core/core.h
        src/server/db/core/users/delete_user.c

        src/server/pkg/config/config.c
        src/server/pkg/config/logger.c
        src/server/pkg/json_parser/json_parser.c
        src/server/pkg/crypto/crypto.c

        src/server/services/service.c
        src/server/services/router.c
        src/server/services/free_request.c

        src/server/services/auth/register.c
        src/server/services/auth/login.c
        src/server/pkg/jwt_utils/jwt_utils.c
        src/server/db/core/users/get_user_by_uuid.c
        src/server/db/core/users/get_user_by_username.c
        src/server/db/core/users/free_user.c
        src/server/pkg/config/logger.c
        src/server/pkg/config/logger.c
        src/server/db/core/users/update_user_username.c
        src/server/db/core/users/update_user_password.c
        src/server/db/core/users/create_user.c
        src/server/db/core/chats/create_chat.c
        src/server/db/core/chats/delete_chat.c
        src/server/db/core/chats/get_chat_by_id.c
        src/server/db/core/chats/update_chat_name.c
        src/server/db/core/chat_members/add_chat_member.c
        src/server/db/core/chat_members/get_chat_members.c
        src/server/db/core/chat_members/is_user_in_chat.c
        src/server/db/core/chat_members/remove_chat_member.c
        src/server/db/core/messages/get_messages_by_chat.c
        src/server/db/core/messages/get_message_by_id.c
        src/server/db/core/messages/delete_message.c
        src/server/db/core/messages/create_message.c
        src/server/db/core/chats/chat_exists.c
        src/server/pkg/httputils/start_with.c
        src/server/services/messenger/chat_group_create.c
        src/server/services/messenger/message_edit.c
        src/server/services/messenger/message_send.c
        src/server/services/messenger/message_edit.c
        src/server/services/messenger/chat_delete.c
        src/server/services/messenger/chat_delete.c
        src/server/services/messenger/message_delete.c
        src/server/services/messenger/message_delete.c
        src/server/services/auth/update_username.c
        src/server/services/auth/logout.c
        src/server/db/core/messages/message_exists.c
        src/server/db/core/messages/edit_message.c
        src/server/pkg/httputils/process_request_data.c
        src/server/services/messenger/chat_private_create.c
        src/server/db/core/chats/chat_private_exist.c
        src/server/services/messenger/chat_group_add_member.c
        src/server/db/core/chat_members/is_user_admin.c
        src/server/services/messenger/chat_group_kick_user.c
        src/server/db/core/chat_members/delete_user_from_chat.c
        src/server/services/messenger/chat_group_leave.c
        src/server/db/core/users/check_user_credentials.c
)

target_link_libraries(uchat_auth yaml pq microhttpd crypto json-c sodium jwt)