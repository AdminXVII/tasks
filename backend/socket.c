#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "backend.h"

task tasks[MAX_TASKS];
task failed[MAX_TASKS];
short nb_tasks = 0;

void remove_task(short idx){
    task *ptr = tasks + idx;
    for (short i = idx + 1; i < nb_tasks; i++){
        *ptr = *(ptr+1);
        ptr++;
    }
    nb_tasks--;
}

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
    strcpy(server.sun_path, "/var/run/task-tracker/backend.sock");
    unlink(server.sun_path);
    if (bind(listenfd, (SA *)&server, strlen(server.sun_path) + sizeof(server.sun_family)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, MAX_TASKS) < 0)
        return -1;
    if (listenfd < 0) {
        perror("Cant open port for internal communication");
        exit(0);
    }
    return listenfd;
}

void parse_json(int fd){
    buf_write(fd, "[[", 2);
    short i = 0;
    for (; i < nb_tasks; i++){
        if (!tasks[i].running)
            continue;
        if (i > 0)
            buf_write(fd, ",", 1);
        buf_write(fd, "[\"", 2);
        buf_write(fd, tasks[i].name, strlen(tasks[i].name));
        buf_write(fd, "\",\"", 3);
        buf_write(fd, tasks[i].msg, strlen(tasks[i].msg));
        buf_write(fd, "\"]", 2);
    }
    buf_write(fd, "],[", 3);
    for (i--; i >= 0; i--){
        if (tasks[i].running)
            continue;
        if (i+1 < nb_tasks)
            buf_write(fd, ",", 1);
        buf_write(fd, "[\"", 2);
        buf_write(fd, tasks[i].name, strlen(tasks[i].name));
        buf_write(fd, "\",\"", 3);
        buf_write(fd, tasks[i].msg, strlen(tasks[i].msg));
        buf_write(fd, "\"]", 2);
        remove_task(i);
    }
    buf_write(fd, "]]", 2);
}

void IPC(int listenfd) {
    struct sockaddr_un client;
    socklen_t len = sizeof client;
    int connfd;

    connfd = accept(listenfd, (SA *)&client, &len);
    task *tsk = tasks+nb_tasks;
    tsk->fd = connfd;
    pthread_create(&(tsk->thread), NULL, update, (void *)tsk);
}

void *update(void *tsk_ptr){
    short len;
    task *tsk = (task *)tsk_ptr;
    nb_tasks++;

    if((len = recv(tsk->fd, &(tsk->name), sizeof(tsk->name), 0)) <= 0){ 
        perror("Closing child");
        exit(0);
    }
    tsk->name[len-1] = '\0'; // Add end-of-string
    tsk->running = true;
    while(1){
        len = recv(tsk->fd, &(tsk->msg), sizeof(tsk->msg), 0);
        if(len == 0){
            tsk->running = false;
            return 0;
        } else if(len < 0){ 
            perror("Closing child");
            exit(0);
        }
        tsk->msg[len-1] = '\0';
    }
}

