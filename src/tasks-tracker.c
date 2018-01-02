#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <systemd/sd-daemon.h>
#include <sys/socket.h>
#include <unistd.h>
#include "backend.h"

int main(int argc, char** argv){
    init_SSL();
    load_conf();
    open_ports();

    // Ignore SIGPIPE signal, so if browser cancels the request, it
    // won't kill the whole process.
    signal(SIGPIPE, SIG_IGN);
    // TODO: add SIGHUP handler

    short nb_child = sysconf(_SC_NPROCESSORS_ONLN); // one child per CPU core
    pthread_t childs[nb_child];
    for(int i = 0; i < nb_child; i++) {
        pthread_create((childs+i), NULL, respond, NULL);
    }
    
    while(1)
        IPC();
    return 0;
}

void load_conf(){
    /*
    config list:
        + certificate
        + port             }
        + public-local     } in systemd config
        + max connections  }
    */
    // set defaults
    char *cert = "/etc/ssl/certs/tasks.pem";
    load_ctx(cert);
}

void open_ports(){
    // receive sockets from systemd
    int nb_fds = sd_listen_fds(0);
    if (nb_fds > 2){
        fprintf(stderr, "Wrong number of file descriptors received.\n");
        exit(0);
    } else if (nb_fds == 0){
        IPC_fd = open_IPC();
        net_fd = open_net();
        if (IPC_fd < 0 || net_fd < 0){
            perror("Error while binding to socket");
            exit(0);
        }
    } else if (sd_is_socket(SD_LISTEN_FDS_START,AF_LOCAL,SOCK_SEQPACKET,-1)){ // order sockets
        IPC_fd = SD_LISTEN_FDS_START;
        net_fd = SD_LISTEN_FDS_START + 1;
    } else {
        IPC_fd = SD_LISTEN_FDS_START + 1;
        net_fd = SD_LISTEN_FDS_START;
    }
}