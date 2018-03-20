#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <event2/event.h>
#include <event2/listener.h>
#include "backend.h"

bool uids[256]; // 256 defines the max concurrent tasks. The max should never be attained, which lets sufficient space to perform round-robin
byte last_uid = -1; // to implement round-robin UID attribution, which is faster than to perform a search

int open_IPC(char *path){
    int optval = 1;
    int listenfd;
    
    struct sockaddr_un server;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_LOCAL, SOCK_SEQPACKET | SOCK_NONBLOCK, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                (const void *)&optval , sizeof(optval)) < 0)
        return -1;

    server.sun_family = AF_LOCAL;
    strcpy(server.sun_path, path);
    unlink(server.sun_path);
    if (bind(listenfd, (SA *)&server, strlen(server.sun_path) + sizeof(server.sun_family)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, SOMAXCONN) < 0)
        return -1;
    return listenfd;
}

void IPC(int fd, char *socket) {
  struct event_base *base = event_base_new();
  
  if (fd == -1) {
    fd = open_IPC(socket);
    if (fd == -1){
      printf("Failed to init the socket");
      exit(1);
    }
  }
  
  evconnlistener_new(base, accept_conn_cb, NULL, 0, 0, fd);
  printf("Listening to incoming tasks on %s\n", socket);
  event_base_dispatch(base);
}

void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {
  /* We got a new connection! Set up an event for it. */
  printf("New listener socket\n");
  event_add(
    event_new(evconnlistener_get_base(listener), fd, EV_READ | EV_PERSIST, &cb_func, new_task()),
    NULL);
}

void accept_error_cb(struct evconnlistener *listener, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    fprintf(stderr, "Got an error on the listener. "
            "Shutting down.\n");

    event_base_loopexit(base, NULL);
}

void cb_func(evutil_socket_t fd, short what, void *arg) {
  task_t *task = arg;
  int len;
  
  message_t *msg = new_message();
  
  if (task->name == NULL) { // first message
    len = recv(fd, task->name, sizeof(task->name), 0);
    task->name[len-1] = '\0';
    msg->len = snprintf(msg->message, "event: new\ndata: {%d:\"%s\"}\n\n", message->len, task->uid, task->name);
  } else {
    len = snprintf(msg->message, "event: msg\ndata: {%d:\"", message->len, task->uid);
    int received_len = recv(fd, (msg->message + len), msg->len - len, 0); // dont't copy two times. This feels hacky, but make the program a lot more performant
    len += received_len;
    if (received_len <= 0) {
      //release(task);
      uids[uid] = false;
    }
    len += snprintf(msg->message + len, "\"}\n\n", message->len - len);
  }
  if (len < 0) {
    //release(task);
    uids[uid] = false;
    perror("Unexpected error while receving");
  }
  
  printf("Got an event on socket %d:%s%s%s%s [%s]",
      (int) fd,
      (what&EV_TIMEOUT) ? " timeout" : "",
      (what&EV_READ)    ? " read" : "",
      (what&EV_WRITE)   ? " write" : "",
      (what&EV_SIGNAL)  ? " signal" : "",
      data);
      
  queue("msg")
}

void *update(void *_fd){
    short len;
    int fd = *(int *)_fd;
    char msg[MAX_MSG_LEN];
    byte uid;
    
    // Round-robin attribution
    do
        last_uid++;
    while(uids[last_uid] == true); // next until empty uid
    uid = last_uid;
    uids[uid] = true;

    len = recv(fd, msg, sizeof(msg), 0);
    if(len < 0){
        uids[uid] = false;
        perror("Error with IPC communication");
        return 0;
    }
    msg[len-1] = '\0';
    sendUpdate(uid, msg, NAME);

    while(1){
    }
}