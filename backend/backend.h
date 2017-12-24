#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdbool.h>
#include <pthread.h>

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

#define MAX_CONN     10   // max connections server-side
#define MAX_TASKS    20   // max concurrent tasks
#define MAX_MSG_LEN  1000 // max message length

typedef enum {
        INVALID,
        GET,
        HEAD
} RequestType;


typedef enum {
        NAME=0,
        MSG=1,
        END=2
} Type;

int main(int argc, char** argv);

//Local side (socket.c)
int openIPC();
void IPC(int listenfd);
void *update(void *_fd);

//Internet side (server.c)
void spawn_server(short port, short nb_child);
void headers(int fd);
void invalid(int fd);
void *receive(void *listenfd);
void sendUpdate(short uid, char *msg, Type type);
RequestType req_type(int fd);
int open_net(int port);

// Helpers (helpers.c)
ssize_t buf_write(int fd, char *buf, size_t n);

#endif
