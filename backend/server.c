#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>          /* inet_ntoa */
#include <errno.h>
#include <pthread.h>
#include "backend.h"

int *conns_fd;
short nb_conn = 0;

char buf[MAX_MSG_LEN+19]; //data: [256,"..."]\n\n\0

int open_net(int port){
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                (const void *)&optval , sizeof(int)) < 0)
        return -1;

    // 6 is TCP's protocol number
    // enable this, much faster : 4000 req/s -> 17000 req/s
    if (setsockopt(listenfd, 6, TCP_CORK,
                (const void *)&optval , sizeof(int)) < 0)
        return -1;

    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, MAX_CONN) < 0)
        return -1;
    return listenfd;
}

void spawn_server(short port, short nb_child){
    int listenfd = open_net(port);
    if (listenfd < 0) {
        perror("Can't open port for internet access");
        exit(listenfd);
    }
    
    conns_fd = calloc(nb_child, sizeof(int));

    pthread_t childs[nb_child];
    for(int i = 0; i < nb_child; i++) {
        pthread_create((childs+i), NULL, receive, (void *)&listenfd);
    }
}

RequestType req_type(int fd){
    char method[5]; // four first char

    read(fd,method,4);
    method[4] = '\0'; // Null-terminator
    if (strcmp(method,"GET ") == 0){
        return GET;
    } else if (strcmp(method,"HEAD") == 0){
        return HEAD;
    }
    return INVALID;
}

void headers(int fd){
    buf_write(fd, "HTTP/1.1 200 OK\r\n"
            "Accept-Ranges: none\r\n"
            "Cache-Control: no-cache\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-type: text/event-stream;\r\n\r\n", 131);
}

void invalid(int fd){
    buf_write(fd, "HTTP/1.1 400 Bad Request\r\n", 27);
}

void sendUpdate(short uid, char *msg, Type type){
    sprintf(buf, "data: [%d,%d,\"%s\"]\n\n", uid, type, msg);
    for (short i = 0; i < nb_conn; i++){
        buf_write(conns_fd[i], buf, strlen(buf));
    }
    // if(type==NAME)
    // store for later use
}

void *receive(void *listenfd){
    short type;
    int fd;

    while(1){
        fd = accept(*(int *)listenfd, NULL, 0);
        type = req_type(fd);

        if(type == HEAD){
            headers(fd);
            shutdown(fd,SHUT_RDWR);
        } else if(type == GET){
            headers(fd);
            // send_names();
            conns_fd[nb_conn] = fd;
            nb_conn++;
        } else {
            invalid(fd);
            shutdown(fd,SHUT_RDWR);
        }
        
    }
}
