#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include "chunk.h"
#include "gencont.h"

#define min(a,b) (((a)>(b))?(b):(a))

chunk_t *chunk_create(int64_t size)
{
    if (size <= 0) {
        errno = EINVAL;
        return NULL;
    }

    chunk_t *chunk = calloc(1, sizeof(chunk_t));
    if (!chunk)
        return NULL;

    chunk->size = size;
    chunk->data = malloc(size);
    if (!chunk->data) {
        free(chunk);
        return NULL;
    }

    int64_t sizeof_int         = sizeof(int);
    int64_t processed_bytes    = 0;
    int64_t available_bytes    = 0;
    int64_t bytes_to_processed = size;
    int     content            = 0;
    char   *curptr             = chunk->data;

    while (processed_bytes < bytes_to_processed) {
        available_bytes = min(sizeof_int, bytes_to_processed - processed_bytes);
        content = rand();
        memcpy(curptr, &content, available_bytes);
        curptr += available_bytes;
        processed_bytes += available_bytes;
    }

    return chunk;
}

void chunk_destroy(chunk_t *chunk)
{
    if (!chunk)
        return;
    
    if (chunk->data)
        free(chunk->data);
    chunk->data = NULL;

    free(chunk);
}

