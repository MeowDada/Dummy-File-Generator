#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

enum UNIT_FORMAT {
    UNIT_FORMAT_NORMAL         = 0,
    UNIT_FORMAT_DETAIL         = 1,
    UNIT_FORMAT_BYTES_ONLY     = 2,
    UNIT_FORMAT_KILOBYTES_ONLY = 3,
    UNIT_FORMAT_MEGABYTES_ONLY = 4,
    UNIT_FORMAT_GIGABYTES_ONLY = 5,
    UNIT_FORMAT_LAST,
};

extern int64_t  unit_to_bytes(const char *);
extern char    *bytes_to_unit(int64_t, int format);

#endif /* UTILS_H */ 