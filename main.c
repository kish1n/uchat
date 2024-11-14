#include <stdio.h>
#include "src/config/config.h"
#include "src/db/db.h"
#include "src/db/migrations.h"

int main() {
    // Загрузка конфигурации
    Config config;
    if (load_config("../config.yaml", &config) != 0) {
        fprintf(stderr, "Ошибка загрузки конфигурации\n");
        return 1;
    }

    // Подключение к базе данных
    PGconn *conn = connect_db(config.database.url);
    if (!conn) {
        return 1;
    }

    // Выполнение миграции
    if (execute_migration(conn, "../src/db/migrations/001_init.sql") != 0) {
        fprintf(stderr, "Ошибка выполнения миграции\n");
    }

    // Закрытие соединения
    disconnect_db(conn);

    return 0;
}
