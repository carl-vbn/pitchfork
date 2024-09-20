#include "stdiobuf.h"

#include <stdio.h>
#include <stdlib.h> 

void stdiobuf_init(stdiobuf_t *buf) {
    buf->buf = malloc(IOBUFSIZE);
    buf->start = buf->buf;
    buf->length = 0;
}

void stdiobuf_dispose(stdiobuf_t *buf) {
    free(buf->buf);
}

void stdiobuf_write(stdiobuf_t *buf, char *data, size_t n) {
    char* p = buf->start + buf->length % IOBUFSIZE;
    for (size_t i = 0; i<n; i++) {
        if ((i || buf->length) && p == buf->start) {
            buf->start++;
            if (buf->start >= buf->buf + IOBUFSIZE) {
                buf->start = buf->buf;
            }
        }

        *(p++) = data[i];

        if (p >= buf->buf + IOBUFSIZE) {
            p = buf->buf;
        }
    }

    buf->length += n;
    if (buf->length > IOBUFSIZE) {
        buf->length = IOBUFSIZE;
    }
}

size_t stdiobuf_read(stdiobuf_t *buf, char *out, size_t n) {
    if (buf->length == 0) return 0;

    size_t to_read = buf->length < n ? buf->length : n;

    if (to_read > IOBUFSIZE) to_read = IOBUFSIZE;

    char *end = buf->start + buf->length % IOBUFSIZE;
    char *p = end - 1;
    for (size_t i = 0; i<to_read; i++) {
        if (p < buf->buf) {
            p = buf->buf + buf->length - 1;
        }

        out[to_read - 1 - i] = *(p--);
    }

    return to_read;
}

void stdiobuf_debug(stdiobuf_t *buf) {
    printf(" 0 |  1 |  2 |  3 |  4 |  5 |  6 | 7\n");
    printf("%x | %x | %x | %x | %x | %x | %x | %x\n", buf->buf[0], buf->buf[1], buf->buf[2], buf->buf[3], buf->buf[4], buf->buf[5], buf->buf[6], buf->buf[7]);
    printf("Start: %ld\tLength: %ld\n", buf->start - buf->buf, buf->length);
}