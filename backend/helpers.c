#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "backend.h"

ssize_t buf_write(int fd, char *buf, size_t n){
    size_t nleft = n;
    ssize_t nwritten;

    while (nleft > 0){
        if ((nwritten = write(fd, buf, nleft)) <= 0){
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