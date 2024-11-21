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
        src/pkg/config/config.c
        main.c
        src/db/core/db.c
        src/db/core/migrations.c
        src/db/core/core.h
        src/pkg/logger/logger.c
        src/db/core/users/users.c
        src/pkg/worker/worker.c
        src/services/service.c
        src/pkg/json_parser/json_parser.c
        src/pkg/crypto/crypto.c
)

target_link_libraries(uchat_auth yaml pq microhttpd crypto json-c)