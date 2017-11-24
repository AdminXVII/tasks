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

char* format_size(unsigned long size){
    char *out = (char*)malloc(13 * sizeof(char));
    if(size < 1024){
        sprintf(out, "%lu", size);
    } else if (size < 1024 * 1024){
        sprintf(out, "%.1fK", (double)size / 1024);
    } else if (size < 1024 * 1024 * 1024){
        sprintf(out, "%.1fM", (double)size / 1024 / 1024);
    } else {
        sprintf(out, "%.1fG", (double)size / 1024 / 1024 / 1024);
    }
    return out;
}
