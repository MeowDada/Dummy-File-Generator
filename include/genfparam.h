#ifndef GENFPARAM_H
#define GENFPARAM_H
#include "stdint.h"

typedef struct param_t {
    char    *filename;
    int64_t  filesize;
    int      fixed_ratio;
    int      non_fixed_ratio;
    int64_t  fixed_part_size;
    int64_t  non_fixed_part_size;
    int64_t  chunk_size;
    int64_t  chunk_size_min;
    int64_t  chunk_size_max;
    int      quiet;
    int      enable_holes;
    int      num_holes;
    int64_t  holes_size;
} param_t;

#endif /* GENFPARAM_H */