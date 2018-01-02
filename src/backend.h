#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdbool.h>
#include <openssl/ssl.h>
#include <string.h>

// The purpose of those variables is to allocate memory. Allocate too much stack (a bit more ram usage) is preferable over allocating from heap (CPU-intensive, slow)
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

typedef struct conn {
    SSL *ssl;
    struct conn *next;
} conn_t;

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

int IPC_fd, net_fd;

int main(int argc, char** argv);
void load_conf();
void open_ports();

//Local side (socket.c)
int open_IPC();
void IPC();
void *update(void *_fd);

//Internet side primitives (server.c)
void spawn_server();
void init_SSL();
void load_ctx(char *cert);
SSL *connect_SSL(int listenfd);
void close_conn(SSL *ssl);
void *respond(void *_);
int open_net();
ssize_t buf_write(SSL *cSSL, char *buf, size_t n);

//Internet HTTP handling (http.c)
RequestType req_type(SSL *cSSL);
int headers(SSL *cSSL);
int invalid(SSL *cSSL);
void sendUpdate(short uid, char *msg, Type type);

#endif
