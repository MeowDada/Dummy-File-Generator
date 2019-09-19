#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "genfile.h"
#include "chunk.h"

#define min(a,b) (((a)>(b))?(b):(a))

static int cmp_int(const void *lhs, const void *rhs)
{
    return *(int *)lhs < *(int *)rhs;
}

static inline int64_t random_chunk_size(int64_t min, int64_t max)
{
    if (max == min)
        return max;
    return min + (rand() % (max-min));
}

static int populate_data_for_fixed_part(FILE *fp, param_t *param)
{
    if (!fp || !param) {
        errno = EINVAL;
        fprintf(stderr, "[ERROR]: populate_data_for_fixed_part: %s\n", strerror(errno));
        return -1;
    }

    if (param->fixed_part_size <= 0)
        return 0;

    int64_t processed_bytes = 0;
    int64_t bytes_to_write  = param->fixed_part_size;
    int64_t available_bytes = 0;
    int64_t written_bytes   = 0;
    int64_t chunksize       = param->chunk_size;
    
    chunk_t *fixed_chunk = chunk_create(chunksize);
    if (!fixed_chunk) {
        fprintf(stderr, "[ERROR]: failed to create fixed chunk\n");
        return -1;
    }

    while (processed_bytes < bytes_to_write) {
        available_bytes = min(bytes_to_write - processed_bytes, chunksize);
        written_bytes   = fwrite(fixed_chunk->data, 1, available_bytes, fp);
        if (available_bytes != written_bytes)
            fprintf(stderr, "[WARN ]: should write %ld bytes, but has wrote %ld bytes", available_bytes, written_bytes);
        processed_bytes += written_bytes;
    }

    free(fixed_chunk);

    return 0;
}

static int populate_data_for_non_fixed_part(FILE *fp, param_t *param)
{
    if (!fp || !param) {
        errno = EINVAL;
        fprintf(stderr, "[ERROR]: populate_data_for_non_fixed_part: %s\n", strerror(errno));
        return -1;
    }

    if (param->non_fixed_part_size <= 0)
        return 0;

    int64_t processed_bytes = 0;
    int64_t bytes_to_write  = param->non_fixed_part_size;
    int64_t available_bytes = 0;
    int64_t written_bytes   = 0;
    int64_t min_chunksize   = param->chunk_size_min;
    int64_t max_chunksize   = param->chunk_size_max;

    chunk_t *chunk = NULL;

    while (processed_bytes < bytes_to_write) {
        int64_t size = random_chunk_size(min_chunksize, max_chunksize);
        chunk = chunk_create(size);
        if (!chunk) {
            fprintf(stderr, "[ERROR]: failed to create random chunk\n");
            return -1;
        }
        available_bytes = min(bytes_to_write - processed_bytes, size);
        written_bytes = fwrite(chunk->data, 1, available_bytes, fp);
        if (written_bytes != available_bytes)
            fprintf(stderr, "[WARN ]: should write %ld bytes, but has wrote %ld bytes", available_bytes, written_bytes);
        processed_bytes += written_bytes;
        chunk_destroy(chunk);
        chunk = NULL;
    }

    return 0;
}

static int append_holes_from_temp_file_to_file(FILE *src, FILE *tar, param_t *param)
{
    if (!src || !tar || !param) {
        errno = EINVAL;
        return -1;
    }

    if (fseek(src, 0, SEEK_SET)) {
        fprintf(stderr, "[ERROR]: failed to fseek the temp file\n");
        return -1;
    }

    if (fseek(tar, 0, SEEK_SET)) {
        fprintf(stderr, "[ERROR]: failed to fseek the target file\n");
        return -1;
    }

    int64_t chunksize            = param->chunk_size;
    int     fixed_ratio          = param->fixed_ratio;
    int64_t fixed_holes_size     = param->holes_size * fixed_ratio / 100;
    int64_t non_fixed_holes_size = param->holes_size - fixed_holes_size;
    int     num_fixed_chunk      = param->fixed_part_size / chunksize;
    int     num_holes            = param->num_holes;
    int     num_holes_fixed      = fixed_ratio * num_holes / 100;
    int     num_holes_non_fixed  = num_holes - num_holes_fixed;

    /* determine where to append holes */
    if (num_holes_fixed > 0) {
        int     *holes_index  = calloc(num_holes_fixed, sizeof(int));
        int      duplicate    = 0;
        int      chunk_idx    = -1;

        memset(holes_index, -1, num_holes_fixed*sizeof(int));

        /* setup holes index */
        for (int i = 0 ; i < num_holes_fixed; i++) {
            duplicate = 0;
            chunk_idx = rand() % num_fixed_chunk;
            for (int j = 0 ; j < i ; j++) {
                if (chunk_idx == holes_index[j]) {
                    i--;
                    duplicate = 1;
                    break;
                }
            }
            if (!duplicate) {
                memcpy(&holes_index[i], &chunk_idx, sizeof(int));
            }
        }
        qsort(holes_index, sizeof(int), num_holes_fixed, cmp_int);

        for (int i = 0 ; i <  num_holes_fixed; i++)
            printf("%d ", holes_index[i]);

        /* copy fixed part and generating holes */
        int64_t copied_fixed_bytes = 0;
        int     fixed_hole_idx     = 0;
        char    buffer[chunksize];
        int64_t hole_size          = fixed_holes_size / num_holes_fixed;
        int64_t last_hole_size     = fixed_holes_size - hole_size * num_holes_fixed;

        for (int i = 0 ; i < num_fixed_chunk; i++) {
            fread(buffer, 1, chunksize, src);
            fwrite(buffer, 1, chunksize, tar);
            if (fixed_hole_idx < num_holes_fixed && holes_index[fixed_hole_idx] == i) {
                if (fixed_hole_idx != num_holes_fixed-1)
                    fseek(tar, hole_size, SEEK_CUR);
                else
                    fseek(tar, last_hole_size, SEEK_CUR);
                fixed_hole_idx++;
            }
            copied_fixed_bytes += chunksize;
        }
        fread(buffer, 1, param->fixed_part_size - copied_fixed_bytes, src);
        fwrite(buffer, 1, param->fixed_part_size - copied_fixed_bytes, tar);
        free(holes_index);
    }
    else {
        int64_t processed_bytes = 0;
        int64_t bytes_to_write  = param->fixed_part_size;
        int64_t written_bytes   = 0;
        int64_t read_bytes      = 0;
        int64_t available_bytes = 0;
        char    buffer[chunksize];

        while (processed_bytes < bytes_to_write) {
            available_bytes = min(chunksize, bytes_to_write-processed_bytes);
            read_bytes = fread(buffer, 1, available_bytes, src);
            if (available_bytes != read_bytes)
                fprintf(stderr, "[WARN ]: should read %ld bytes, but read only %ld bytes\n", available_bytes, read_bytes);
            written_bytes = fwrite(buffer, 1, read_bytes, tar);
            if (read_bytes != written_bytes)
                fprintf(stderr, "[WARN ]: should write %ld bytes, but wrote only %ld bytes instead\n", read_bytes, written_bytes);
            processed_bytes += written_bytes;
        }
    }

    /* copy non fixed part and generating holes */
    if (num_holes_non_fixed > 0) {
        int64_t num_holes_gen = 0;
        int64_t non_fixed_hole_len = non_fixed_holes_size / num_holes_non_fixed;
        int64_t last_hole_size     = non_fixed_holes_size - non_fixed_hole_len * num_holes_non_fixed;
        int64_t copied_non_fixed_bytes = 0;
        int64_t non_fixed_bytes_to_copy = param->non_fixed_part_size;
        int64_t min_chunksize = param->chunk_size_min;
        int64_t max_chunksize = param->chunk_size_max;
        int64_t size = 0;
        int64_t available_bytes = 0;
        int64_t read_bytes      = 0;
        int64_t written_bytes   = 0;
        char    buffer2[max_chunksize];

        while (copied_non_fixed_bytes < non_fixed_bytes_to_copy) {
            if (num_holes_gen < num_holes_non_fixed) {
                if (num_holes_gen != num_holes_non_fixed-1)
                    fseek(tar, non_fixed_hole_len, SEEK_CUR);
                else
                    fseek(tar, last_hole_size, SEEK_CUR);
                num_holes_gen++;
            }
            size = random_chunk_size(min_chunksize, max_chunksize);
            available_bytes = min(non_fixed_bytes_to_copy - copied_non_fixed_bytes, size);
            read_bytes = fread(buffer2, 1, available_bytes, src);
            if (read_bytes != available_bytes)
                fprintf(stderr, "[WARN ]: should read %ld bytes, but only read %ld bytes\n", available_bytes, read_bytes);
            written_bytes = fwrite(buffer2, 1, read_bytes, tar);
            if (written_bytes != read_bytes)
                fprintf(stderr, "[WARN ]: should write %ld bytes, but only wrote %ld bytes instead\n", read_bytes, written_bytes);
            copied_non_fixed_bytes += written_bytes;
        }
    }
    else {
        int64_t processed_bytes = 0;
        int64_t bytes_to_write  = 0;
        int64_t written_bytes   = 0;
        int64_t read_bytes      = 0;
        int64_t available_bytes = 0;
        char    buffer[chunksize];

        while (processed_bytes < bytes_to_write) {
            available_bytes = min(chunksize, bytes_to_write-processed_bytes);
            read_bytes = fread(buffer, 1, available_bytes, src);
            if (available_bytes != read_bytes)
                fprintf(stderr, "[WARN ]: should read %ld bytes, but read only %ld bytes\n", available_bytes, read_bytes);
            written_bytes = fwrite(buffer, 1, read_bytes, tar);
            if (read_bytes != written_bytes)
                fprintf(stderr, "[WARN ]: should write %ld bytes, but wrote only %ld bytes instead\n", read_bytes, written_bytes);
            processed_bytes += written_bytes;
        }
    }

    return 0;
}

static int do_generate_file_with_no_holes(param_t *param)
{
    int error = 0;

    FILE *fp = fopen(param->filename, "wb+");
    if (!fp) {
        fprintf(stderr, "[ERROR]: generate_file: %s\n", strerror(errno));
        return -1;
    }

    /* populate the fixed part with random data */
    if (populate_data_for_fixed_part(fp, param)) {
        fprintf(stderr, "[ERROR]: some errors occur when populate data for fixed part\n");
        error = -1;
        goto cleanup;
    }

    /* populate the non-fixed part with random data */
    if (populate_data_for_non_fixed_part(fp, param)) {
        fprintf(stderr, "[ERROR]: some errors occur when populate data for non fixed part\n");
        error = -1;
        goto cleanup;
    }

cleanup:
    if (fp)
        fclose(fp);
    
    return error;
}

static int do_generate_file_with_holes(param_t *param)
{
    int error = 0;

    FILE *fp = fopen(param->filename, "wb+");
    if (!fp) {
        fprintf(stderr, "[ERROR]: generate_file: %s\n", strerror(errno));
        return -1;
    }

    char temp_file[FILENAME_MAX];
    char ext[] = "tmp";
    if (strlen(param->filename) + strlen(ext) > FILENAME_MAX) {
        fprintf(stderr, "[ERROR]: failed to generate temp file with name exceeed FILENAME_MAX limitation\n");
        error = -1;
        goto cleanup;
    }
    snprintf(temp_file, FILENAME_MAX, "%s.%s", param->filename, ext);
    
    FILE *temp_fp = fopen(temp_file, "wb+");
    if (!temp_fp) {
        fprintf(stderr, "[ERROR]: generate_file: %s\n", strerror(errno));
        error = -1;
        goto cleanup;
    }

    /* populate the fixed part with random data */
    if (populate_data_for_fixed_part(temp_fp, param)) {
        fprintf(stderr, "[ERROR]: some errors occur when populate data for fixed part\n");
        error = -1;
        goto cleanup;
    }

    /* populate the non-fixed part with random data */
    if (populate_data_for_non_fixed_part(temp_fp, param)) {
        fprintf(stderr, "[ERROR]: some errors occur when populate data for non fixed part\n");
        error = -1;
        goto cleanup;
    }

    if (append_holes_from_temp_file_to_file(temp_fp, fp, param)) {
        fprintf(stderr, "[ERROR]: failed to append holes from temp file to target file\n");
        error = -1;
        goto cleanup;
    }

cleanup:
    if (fp)
        fclose(fp);
    if (temp_fp)
        fclose(temp_fp);

    if (remove(temp_file)) {
        fprintf(stderr, "[ERROR]: failed to remove temp file: %s\n", strerror(errno));
        error = -1;
    }
    
    return error;
}

int generate_file(param_t *param)
{
    if (!param) {
        errno = EINVAL;
        return -1;
    }

    if (param->enable_holes)
        return do_generate_file_with_holes(param);
    else
        return do_generate_file_with_no_holes(param);

    return 0;
}