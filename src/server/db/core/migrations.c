#include <stdio.h>
#include <stdlib.h>
#include "core.h"

int execute_migration(PGconn *conn, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening migration file");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *query = (char *)malloc(file_size + 1);
    if (fread(query, 1, file_size, file) != file_size) {
        perror("Error reading migration file");
        free(query);
        fclose(file);
        return -1;
    }
    query[file_size] = '\0';
    fclose(file);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Migration execution error: %s\n", PQerrorMessage(conn));
        PQclear(res);
        free(query);
        return -1;
    }

    printf("Migration from file %s executed successfully\n", filename);

    PQclear(res);
    free(query);
    return 0;
}
