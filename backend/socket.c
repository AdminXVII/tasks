#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "backend.h"

bool uids[256]; // 256 defines the max concurrent tasks. The max should never be attained, which lets sufficient space to perform useful round-robin
unsigned char last_uid = -1; // to implement round-robin UID attribution, which is faster than to perform a search

int openIPC(){
    int listenfd, optval=1;
    struct sockaddr_un server;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_LOCAL, SOCK_SEQPACKET, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                (const void *)&optval , sizeof(optval)) < 0)
        return -1;

    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    server.sun_family = AF_LOCAL;  /* local is declared before socket() ^ */
    strcpy(server.sun_path, "/tmp/task-tracker.sock");
    unlink(server.sun_path);
    if (bind(listenfd, (SA *)&server, strlen(server.sun_path) + sizeof(server.sun_family)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, MAX_TASKS) < 0)
        return -1;
    if (listenfd < 0)
        return -1;
    return listenfd;
}

void IPC(int listenfd) {
    struct sockaddr_un client;
    socklen_t len = sizeof client;
    int connfd;
    pthread_t thread;

    connfd = accept(listenfd, (SA *)&client, &len);
    if(connfd < 0){
            perror("Error binding to socket");
            exit(0);
    }
    pthread_create(&thread, NULL, update, (void *)&connfd);
}

void *update(void *_fd){
    short len;
    int fd = *(int *)_fd;
    char msg[MAX_MSG_LEN];
    unsigned char uid;
    
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
        len = recv(fd, msg, sizeof(msg), 0);
        if(len == 0){
            sendUpdate(uid, NULL, END);
            uids[uid] = false;
            return 0;
        } else if(len < 0){
            sendUpdate(uid, msg, END);
            uids[uid] = false;
            perror("Closing IPC child");
            return 0;
        }
        msg[len-1] = '\0';
        sendUpdate(uid, msg, MSG);
    }
}