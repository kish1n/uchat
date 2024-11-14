cmake_minimum_required(VERSION 3.29)
project(uchat_auth C)

set(CMAKE_C_STANDARD 11)

# Найти библиотеку yaml
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.1)

include_directories(${YAML_INCLUDE_DIRS} /usr/include/postgresql)
link_directories(/usr/lib/x86_64-linux-gnu)

add_executable(uchat_auth
        src/config/config.c
        main.c
        src/db/db.c
        src/db/migrations.h
        src/db/migrations.c
        src/db/db.h
)

target_link_libraries(uchat_auth ${YAML_LIBRARIES} pq)
