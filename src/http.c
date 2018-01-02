#include <openssl/ssl.h>
#include "backend.h"

char buf[MAX_MSG_LEN+17]; // data: [256,"..."]\n\n\0   => except the message, the format takes 14 meta character and 3 digits at most
extern conn_t *conns;

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
    conn_t *conn, *prev;
    
    if (type == END){
        sprintf(buf, "data: [%d,%d]\n\n", uid, type);
    } else {
        sprintf(buf, "data: [%d,%d,\"%s\"]\n\n", uid, type, msg);
    }
    conn = conns;
    prev = NULL;
    while(conn->next != NULL){
        if(buf_write(conn->ssl, buf, strlen(buf)) == -1){
            close_conn(conn->ssl);
            if(prev != NULL)
                prev->next = conn->next;
            free(conn);
        }
        prev = conn;
        conn = conn->next;
    }
    // if(type==NAME)
    // store for later use
}
