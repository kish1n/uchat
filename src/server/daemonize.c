#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void daemonize(int port) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("First fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Мы в потомке первого форка
    if (setsid() < 0) {
        perror("Failed to create new session");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0) {
        perror("Second fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Мы в окончательном демоне (потомке второго форка)
    // Логируем PID итогового демона
    logging(INFO, "Final daemon PID: %d, Listening on port: %d", getpid(), port);

    // Перенаправляем дескрипторы
    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null < 0) {
        perror("Failed to open /dev/null");
        exit(EXIT_FAILURE);
    }

    dup2(dev_null, STDIN_FILENO);
    dup2(dev_null, STDOUT_FILENO);
    dup2(dev_null, STDERR_FILENO);

    if (dev_null > STDERR_FILENO) {
        close(dev_null);
    }

    // Далее идёт основная работа демона
}
