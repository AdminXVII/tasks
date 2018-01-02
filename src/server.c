//#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>          /* inet_ntoa */
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "backend.h"

conn_t *conns; // SSL sessions

SSL_CTX *sslctx;

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
    if (listen(listenfd, SOMAXCONN) < 0)
        return -1;
    return listenfd;
}

void init_SSL(){
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

void close_conn(SSL *cSSL){
    int fd = SSL_get_fd(cSSL);
    SSL_free(cSSL);
    close(fd);
}

void load_ctx(char *cert){
    SSL_CTX *ctx;
    
    sslctx = SSL_CTX_new( TLS_server_method());
    SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);
    int use_cert = SSL_CTX_use_certificate_file(sslctx, cert , SSL_FILETYPE_PEM);
    int use_prv = SSL_CTX_use_PrivateKey_file(sslctx, cert, SSL_FILETYPE_PEM);
    
    if (use_cert != 1 || use_prv != 1){
        perror("Error while loading certificate");
        exit(0);
    }
    sslctx = ctx;
}

void *respond(void *_){
    short type;
    SSL *cSSL;
    struct sockaddr_in client;
    socklen_t len = sizeof client;

    while(1){
        int fd = accept(net_fd, (SA *)&client, &len);
        if (fd < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

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
        
        if(type == HEAD){
            if (headers(cSSL) < 0)
                perror("Error while sending headers");
            close_conn(cSSL);
        } else if(type == GET){
            if (headers(cSSL) < 0){
                perror("Error while sending headers");
                close_conn(cSSL);
            }
            conn_t *conn = malloc(sizeof(conn_t));
            if (conn == NULL) // cannot afford another connection, so cancel it
                close_conn(cSSL);
            conn->next = conns;
            conn->ssl = cSSL;
            conns = conn;
            // send_names();
        } else {
            invalid(cSSL);
            close_conn(cSSL);
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