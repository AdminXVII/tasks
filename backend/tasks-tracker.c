#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "backend.h"


int main(int argc, char** argv){
    short port = 9000, nb_child = MAX_CONN;
    if(argc == 2) {
        if(argv[1][0] >= '0' && argv[1][0] <= '9') {
            port = atoi(argv[1]);
        } else {
            printf("Usage: backend [port]\n\n");
            exit(1);
        }
    }

    // Ignore SIGPIPE signal, so if browser cancels the request, it
    // won't kill the whole process.
    signal(SIGPIPE, SIG_IGN);
    
    // Become a daemon
    pid_t pid = fork();
    if (pid == -1) {
        perror("failed to fork while daemonising");
    } else if (pid != 0) {
        _exit(0);
    }

    int fd = openIPC();
    if (fd < 0){
        perror("Error while binding to socket");
        exit(0);
    }
    spawn_server(port, nb_child);
    while(1)
        IPC(fd);
    return 0;
}
