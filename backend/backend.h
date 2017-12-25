#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdbool.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

// The purpose of those variables is to allocate memory. Allocate too much stack (a bit more ram usage) is preferable over allocating from heap (CPU-intensive, slow)
#define MAX_CONN     10   // max connections server-side
#define MAX_TASKS    20   // max concurrent tasks
#define MAX_MSG_LEN  1000 // max message length
#define MAX_NAME_LEN 100  // max name length

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
void init_SSL();
SSL *connect_SSL(int listenfd);
void close_conn(short conn);
void *receive(void *listenfd);
int open_net(int port);
ssize_t buf_write(SSL *cSSL, char *buf, size_t n);

RequestType req_type(SSL *cSSL);
int headers(SSL *cSSL);
int invalid(SSL *cSSL);
void sendUpdate(short uid, char *msg, Type type);

#endif
