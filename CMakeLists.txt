cmake_minimum_required(VERSION 3.29)
project(uchat_auth C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.1)

# Найти библиотеку OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${YAML_INCLUDE_DIRS} /usr/include/postgresql)
link_directories(/usr/lib/x86_64-linux-gnu)

file(GLOB AUTH_HANDLERS_SRC "src/services/auth/handlers/*.c")

add_executable(uchat_auth
        main.c

        src/db/core/db.c
        src/db/core/migrations.c
        src/db/core/core.h
        src/db/core/users/users.c

        src/pkg/config/config.c
        src/pkg/config/logger.c
        src/pkg/json_parser/json_parser.c
        src/pkg/crypto/crypto.c

        src/services/service.c
        src/services/handle_request.c
        src/services/free_request.c

        src/services/auth/register.c
        src/services/auth/login.c
        src/pkg/jwt_utils/jwt_utils.c
        src/db/core/users/get_user_by_uuid.c
        src/db/core/users/get_user_by_username.c
        src/db/core/users/free_user.c
        src/pkg/config/logger.c
        src/pkg/config/logger.c
)

target_link_libraries(uchat_auth yaml pq microhttpd crypto json-c sodium jwt)