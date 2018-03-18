#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <systemd/sd-daemon.h>
#include <sys/socket.h>
#include <unistd.h>
#include "backend.h"

int main(int argc, char** argv){
  int IPC_fd, net_fd;
  options_t opts;

  // Ignore SIGPIPE signal, so if browser cancels the request, it
  // won't kill the whole process.
  signal(SIGPIPE, SIG_IGN);
  // TODO: add SIGHUP handler
  
  // Load configuration
  if (!systemd(&IPC_fd, &net_fd)) {
    load_conf(&opts);
    if (IPC_fd == -1)
      IPC_fd = open_IPC(opts.socket);
  }
  printf("Loaded config\n");
  
  // Start server
  if (spawn_server(net_fd, opts.port) == MHD_NO){
    printf("Failed to launch server\n");
    return 1;
  }
  printf("Launched server\n");
  
  // Start server
  IPC(IPC_fd, opts.socket);
  return 0;
}

void load_conf(options_t *opts){
  /*
  config list:
      + internet port    }
      + unix socket      } in systemd config
      + public-local     }
  */
  // set defaults
  opts->socket = "/tmp/tasks.sock";
  opts->port = 1234;
  opts->is_public = false;
}

int systemd(int *IPC_fd, int *net_fd){
    // receive sockets from systemd
    switch (sd_listen_fds(0)) {
      case 0:
        *IPC_fd = -1;
        *net_fd = -1;
        return 0;
        
      case 1:
        if (sd_is_socket(SD_LISTEN_FDS_START, AF_LOCAL,SOCK_SEQPACKET, -1)){ // order sockets
          *IPC_fd = SD_LISTEN_FDS_START;
          *net_fd = -1;
        } else {
          *IPC_fd = -1;
          *net_fd = SD_LISTEN_FDS_START;
        }
        return 0;
        
      case 2:
        if (sd_is_socket(SD_LISTEN_FDS_START, AF_LOCAL,SOCK_SEQPACKET, -1)){ // order sockets
          *IPC_fd = SD_LISTEN_FDS_START;
          *net_fd = SD_LISTEN_FDS_START + 1;
        } else {
          *IPC_fd = SD_LISTEN_FDS_START + 1;
          *net_fd = SD_LISTEN_FDS_START;
        }
        return 1;
        
      default:
        fprintf(stderr, "Too many file descriptors received.\n");
        exit(0);
    }
}