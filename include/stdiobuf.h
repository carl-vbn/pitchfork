#ifndef _STDIOBUF_H
#define _STDIOBUF_H

#include <stddef.h>

// The size of the buffer used to store process IO for access by control sockets
#define IOBUFSIZE 8192

typedef struct stdiobuf_s {
    char *buf;
    char *start;
    size_t length;
} stdiobuf_t;

void stdiobuf_init(stdiobuf_t *buf);
void stdiobuf_dispose(stdiobuf_t *buf);
void stdiobuf_write(stdiobuf_t *buf, char *data, size_t n);
size_t stdiobuf_read(stdiobuf_t *buf, char *out, size_t n);
void stdiobuf_debug(stdiobuf_t *buf);

#endif