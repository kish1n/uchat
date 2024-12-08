#ifndef SERVER_H
#define SERVER_H

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include "pkg/config/config.h"

typedef struct {
    int port;
    struct MHD_Daemon *daemon;
} Server;

Server *server_init(int port);
int server_start(Server *server);
void server_stop(Server *server);
void server_destroy(Server *server);

void daemonize();
void signal_handler(int sig);
int main(int argc, char *argv[]);

#endif