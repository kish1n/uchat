#include "server.h"

void daemonize(int port) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        logging(INFO, "Parent PID: %d, Listening on port: %d", pid, port);
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        perror("Failed to create new session");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        logging(INFO, "Daemon PID: %d, Listening on port: %d", pid, port);
        exit(EXIT_SUCCESS);
    }

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
}
