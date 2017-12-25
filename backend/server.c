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
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "backend.h"

SSL *conns[MAX_TASKS]; // SSL sessions
short nb_conn = 0;

SSL_CTX *sslctx;

char buf[MAX_MSG_LEN+17]; // data: [256,"..."]\n\n\0   => excepted the message, the format takes 14 meta character and 3 digits at most

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

void init_SSL(){
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

void close_conn(short conn){
    int fd = SSL_get_fd(conns[conn]);
    
    SSL_free(conns[conn]);
    close(fd);
    nb_conn--;
    for (short i = conn; i < nb_conn; i++)
        conns[i] = conns[i+1];
}

SSL_CTX *context(){
    SSL_CTX *sslctx;
    
    sslctx = SSL_CTX_new( TLS_server_method());
    SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);
    int use_cert = SSL_CTX_use_certificate_file(sslctx, "/etc/ssl/certs/server.pem" , SSL_FILETYPE_PEM);
    int use_prv = SSL_CTX_use_PrivateKey_file(sslctx, "/etc/ssl/certs/server.key", SSL_FILETYPE_PEM);
    
    if (use_cert != 1 || use_prv != 1){
        perror("Error while loading certificate");
        exit(0);
    }
    return sslctx;
}

void spawn_server(short port, short nb_child){
    init_SSL();
    int listenfd = open_net(port);
    if (listenfd < 0) {
        perror("Can't open port for internet access");
        exit(0);
    }
    
    sslctx = context();

    pthread_t childs[nb_child];
    for(int i = 0; i < nb_child; i++) {
        pthread_create((childs+i), NULL, receive, (void *)&listenfd);
    }
}

RequestType req_type(SSL *cSSL){
    char method[5]; // four first char

    if (SSL_read(cSSL,method,4) < 0)
        return -1;
    method[4] = '\0'; // Null-terminator
    if (strcmp(method,"GET ") == 0){
        return GET;
    } else if (strcmp(method,"HEAD") == 0){
        return HEAD;
    }
    return INVALID;
}

int headers(SSL *cSSL){
    return buf_write(cSSL, "HTTP/1.1 200 OK\r\n"
            "Accept-Ranges: none\r\n"
            "Cache-Control: no-cache\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-type: text/event-stream;\r\n\r\n", 131);
}

int invalid(SSL *cSSL){
    return buf_write(cSSL, "HTTP/1.1 400 Bad Request\r\n", 27);
}

void sendUpdate(short uid, char *msg, Type type){
    if (type == END){
        sprintf(buf, "data: [%d,%d]\n\n", uid, type);
    } else {
        sprintf(buf, "data: [%d,%d,\"%s\"]\n\n", uid, type, msg);
    }
    for (short i = 0; i < nb_conn; i++){
        if (conns[i] == NULL)
            continue;
        if(buf_write(conns[i], buf, strlen(buf)) == -1)
            close_conn(i);
    }
    // if(type==NAME)
    // store for later use
}

void *receive(void *listenfd){
    short type;
    SSL *cSSL;
    struct sockaddr_in client;
    socklen_t len = sizeof client;

    while(1){
        int fd = accept(*(int *)listenfd, (SA *)&client, &len);
        if (fd < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }
        short conn_no = nb_conn;
        nb_conn++;

        cSSL = SSL_new(sslctx);
        if (cSSL == NULL){
            perror("Error at SSL initialization");
            continue;
        }
        if (SSL_set_fd(cSSL, fd) == 0)
            perror("Error while setting file descriptor");
        if (SSL_accept(cSSL) <= 0) {
            perror("Cannot accept SSL connection");
        }
        type = req_type(cSSL);
        conns[conn_no] = cSSL;
        
        if(type == HEAD){
            if (headers(cSSL) < 0)
                perror("Error while sending headers");
            close_conn(conn_no);
        } else if(type == GET){
            if (headers(cSSL) < 0){
                perror("Error while sending headers");
                close_conn(conn_no);
            }
            // send_names();
        } else {
            invalid(cSSL);
            close_conn(conn_no);
        }
    }
}

// Helper
ssize_t buf_write(SSL *cSSL, char *buf, size_t n){
    size_t nleft = n;
    ssize_t nwritten;

    while (nleft > 0){
        if ((nwritten = SSL_write(cSSL, buf, nleft)) <= 0){
            if (errno == EINTR)  /* interrupted by sig handler return */
                nwritten = 0;    /* and call write() again */
            else
                return -1;       /* errorno set by write() */
        }
        nleft -= nwritten;
        buf += nwritten;
    }
    return n;
}