#include <fcntl.h>
#include "service.h"

void daemonize(void) {
    pid_t pid = 0;
    pid_t sid = 0;

    pid = fork();
    if (pid < 0) {
        logging(ERROR, "Fork failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        logging(INFO, "Daemon started with PID: %d", pid);
        exit(EXIT_SUCCESS);
    }

    sid = setsid();
    if (sid < 0) {
        logging(ERROR, "Failed to create new session: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (chdir("/") < 0) {
        logging(ERROR, "Failed to change directory to /: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    close (STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    if (open("/dev/null", O_RDONLY) < 0) {
        logging(WARN, "Failed to redirect stdin to /dev/null: %s", strerror(errno));
    }
    if (open("/dev/null", O_RDWR) < 0) {
        logging(WARN, "Failed to redirect stdout to /dev/null: %s", strerror(errno));
    }
    if (open("/dev/null", O_RDWR) < 0) {
        logging(WARN, "Failed to redirect stderr to /dev/null: %s", strerror(errno));
    }
}

void signal_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        logging(INFO, "Received termination signal. Shutting down...");
        exit(EXIT_SUCCESS);
    }
}

