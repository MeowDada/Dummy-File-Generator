#ifndef CHUNK_H
#define CHUNK_H
#include <stdint.h>

typedef struct chunk_t {
    int64_t size;
    char *data;
} chunk_t;

extern chunk_t *chunk_create(int64_t size);
extern void     chunk_destroy(chunk_t *chunk);

#endif /* CHUNK_H */