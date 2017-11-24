#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdbool.h>
#include <pthread.h>

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

#define MAX_CONN     1024 // max connections server-side
#define MAX_TASKS    20   // max concurrent tasks
#define MAX_MSG_LEN  1000 // max message length

typedef enum {
        INVALID,
        GET,
        HEAD
} RequestType;

typedef struct{
    char name[50];
    char msg[MAX_MSG_LEN];
    bool running;
    int fd;
    pthread_t thread;
} task;

int main(int argc, char** argv);

//Local side (socket.c)
int open_unix(int port);
void parse_json(int fd);
int spawn_IPC(short port, short nb_child);
void IPC(int listenfd);
void *update(void *tsk_ptr);

//Internet side (server.c)
void spawn_server(short port, short nb_child);
void headers(int fd);
void invalid(int fd);
void *receive(void *listenfd);
RequestType req_type(int fd);
int open_net(int port);

// Helpers (helpers.c)
char* format_size(unsigned long size);
ssize_t buf_write(int fd, char *buf, size_t n);

#endif
