#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "backend.h"


int main(int argc, char** argv){
    short port = 80, nb_child = 10;
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

    int fd = spawn_IPC(256, 20);
    spawn_server(port, nb_child);
    while(1)
        IPC(fd);
    return 0;
}
