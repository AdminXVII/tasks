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
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
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

    pthread_t childs[nb_child];
    for(int i = 0; i < nb_child; i++) {
        pthread_create(&(childs[i]), NULL, receive, (void *)&listenfd);
    }
}

RequestType req_type(int fd){
    char method[4]; // four first char

    read(fd,method,4);
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
            "Content-type: application/json;\r\n\r\n", 130);
}

void invalid(int fd){
    printf("Invalid request\n");
    buf_write(fd, "HTTP/1.1 400 Bad Request\r\n", 27);
}

void *receive(void *listenfd){
    short type;
    int fd;

    while(1){
        fd = accept(*(int *)listenfd, NULL, 0);
        type = req_type(fd);

        if(type == GET || type == HEAD){
            headers(fd);
        } else {
            invalid(fd);
        }

        if(type == GET)
            parse_json(fd);
        shutdown(fd, SHUT_WR);
    }
}
