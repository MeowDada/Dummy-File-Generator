#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "utils.h"

typedef char unit_format(int64_t);

#define BYTES_TO_KILOBYTES (1024LL)
#define BYTES_TO_MEGABYTES (1048576LL)
#define BYTES_TO_GIGABYTES (1073741824LL)

#define UNIT_FORMAT_NORMAL_PRECISION 2

static char *unit_format_normal(int64_t _bytes)
{
    int64_t bytes = _bytes;
    int     sign  = 1;
    char    buf[64];

    if (bytes < 0) {
        bytes *=  1;
        sign   = -1;
    }

    int64_t prec = 1;
    for (int i = 0; i < UNIT_FORMAT_NORMAL_PRECISION; i++)
        prec *= 10;

    if (bytes < BYTES_TO_KILOBYTES) {
        snprintf(buf, 64, "%ld bytes", bytes);
    }
    else if (bytes < BYTES_TO_MEGABYTES) {
        int64_t val = bytes * prec / BYTES_TO_KILOBYTES;
        int64_t precision = val % prec;
        val /= prec;
        snprintf(buf, 64, "%ld.%ld KB", val, precision);
    }
    else if (bytes < BYTES_TO_GIGABYTES) {
        int64_t val = bytes * prec / BYTES_TO_MEGABYTES;
        int64_t precision = val % prec;
        val /= prec;
        snprintf(buf, 64, "%ld.%ld MB", val, precision);
    }
    else {
        int64_t val = bytes * prec / BYTES_TO_GIGABYTES;
        int64_t precison = val % prec;
        val /= prec;
        snprintf(buf, 64, "%ld.%ld GB", val, precison);
    }

    if (sign == -1) {
        snprintf(buf, 64, "-%s", buf);
    }

    return strdup(buf);
}

static char *unit_format_detail(int64_t _bytes)
{
    char buf[128];

    int64_t bytes = _bytes;
    int     sign  = 1;
    if (bytes < 0) {
        sign = -1;
        bytes = -bytes;
    }

    int64_t gb    = bytes / BYTES_TO_GIGABYTES;
    bytes -= ( gb * BYTES_TO_GIGABYTES );
    int64_t mb    = bytes / BYTES_TO_MEGABYTES;
    bytes -= ( mb * BYTES_TO_MEGABYTES );
    int64_t kb    = bytes / BYTES_TO_KILOBYTES;
    bytes -= ( kb * BYTES_TO_KILOBYTES );
    int64_t b     = bytes;

    if (sign == 1)
        snprintf(buf, 128, "%ld GB, %ld MB, %ld KB, %ld bytes", gb, mb, kb, b);
    else if (sign == -1)
        snprintf(buf, 128, "- %ld GB, %ld MB, %ld KB, %ld bytes", gb, mb, kb, b);
    return strdup(buf);
}

static unit_format *unit_format_multiplexor[] = {
    &unit_format_normal,
    &unit_format_detail,
};

int64_t unit_to_bytes(const char *str)
{
    if (!str) {
        errno = EINVAL;
        fprintf(stderr, "unit_to_bytes: %s\n", strerror(errno));
        return -1;
    }

    int64_t num    = 0;
    char buf[8];

    sscanf(str, "%ld%s", &num, buf);

    if (strcmp(buf, "")  == 0 ||
        strcmp(buf, "B") == 0 ||
        strcmp(buf, "b") == 0) {
        return num;
    }
    else if (strcmp(buf, "KB") == 0 ||
        strcmp(buf, "kb") == 0 ||
        strcmp(buf, "k")  == 0 ||
        strcmp(buf, "K")  == 0) {
        return num * BYTES_TO_KILOBYTES;
    }
    else if (strcmp(buf, "MB") == 0 ||
        strcmp(buf, "mb") == 0 ||
        strcmp(buf, "m")  == 0 ||
        strcmp(buf, "M")  == 0) {
        return num * BYTES_TO_MEGABYTES;
    }
    else if (strcmp(buf, "GB") == 0 ||
        strcmp(buf, "gb") == 0 ||
        strcmp(buf, "g")  == 0 ||
        strcmp(buf, "G")  == 0) {
        return num * BYTES_TO_GIGABYTES;
    }
    else {
        fprintf(stderr, "unsupported unit\n");
        return -1;
    }
    return 0;
}

char *bytes_to_unit(int64_t bytes, int format)
{
    if (format < 0 || format >= UNIT_FORMAT_LAST) {
        errno = EINVAL;
        fprintf(stderr, "bytes_to_unit: Unsupported format type\n");
        return NULL;
    }

    return (*unit_format_multiplexor[format]);
}