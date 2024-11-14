#include <stdio.h>
#include <stdlib.h>
#include "migrations.h"

// Функция для выполнения миграции из файла SQL
int execute_migration(PGconn *conn, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Ошибка открытия файла миграции");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *query = (char *)malloc(file_size + 1);
    if (fread(query, 1, file_size, file) != file_size) {
        perror("Ошибка чтения файла миграции");
        free(query);
        fclose(file);
        return -1;
    }
    query[file_size] = '\0';
    fclose(file);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Ошибка выполнения миграции: %s\n", PQerrorMessage(conn));
        PQclear(res);
        free(query);
        return -1;
    }

    printf("Миграция из файла %s выполнена успешно\n", filename);

    PQclear(res);
    free(query);
    return 0;
}
