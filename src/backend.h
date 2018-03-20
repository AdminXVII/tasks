#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdbool.h>
#include <string.h>
#include <microhttpd.h>
#include <event2/event.h>
#include <event2/listener.h>

// The purpose of those variables is to allocate memory. Allocating too much stack (a bit more ram usage) is preferable over allocating from heap (CPU-intensive and slow), so we predefine max size to allocate on the stack
#define MAX_MSG_LEN  1000 // max message length
#define MAX_NAME_LEN 100  // max name length

/* type of uid. uid_t was already defined so opted toward byte because UIDs are effectively requesting one byte */
typedef unsigned char byte;

/* Type of message sent */
typedef enum {
        NAME=0,
        MSG=1,
        END=2
} message_type;

/* Defines the options of the server */
/* Thread-safety: Accessed by only one thread */
typedef struct {
  char *socket;
  int port;
  bool is_public;
} options_t;

/* A task */
/* Thread safety: written by only once. The lock keeps the writer from clearing data while it is being read */
typedef struct {
  char name[MAX_NAME_LEN];
  byte uid;
} task_t;

/* Message from a stream */
/* Thread-safety: no write during use. Gets cleared when the reference counter gets to zero. */
typedef struct {
  char *message;
  int len; // in uninitialized datastructure, this represent the maximum length. After, it represent the string length
  struct message_t *next;
  short references;
} message_t;

/* Position in a stream. Needed because only one argument is allowed in MicroHTTPD callbacks */
/* Thread-Safety: only one thread updates the variable at a time */
typedef struct {
  message_t *message;
  int offset;
} position_t;

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

int main(int argc, char** argv);
void load_conf(options_t *opts);
int systemd(int *IPC_fd, int *net_fd);

//Local side (socket.c)
int open_IPC(char *socket);
void IPC(int fd, char *socket);
void *update(void *_fd);
void cb_func(evutil_socket_t fd, short what, void *arg);
void accept_error_cb(struct evconnlistener *listener, void *ctx);
void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx);

//Internet HTTP handling (http.c)
void sendUpdate(byte uid, char *msg, message_type type);
int spawn_server(int fd, int port);
int init_connection(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method,
                         const char* version, const char* upload_data,
                         size_t* upload_data_size, void** con_cls);
int get_data (void *cls, uint64_t pos, char *buf, size_t max);
void cancel_client (void *cls);
position_t *new_stream();

#endif
