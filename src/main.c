#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include "genfile.h"
#include "utils.h"

#define DEFAULT_FIXED_RATIO 20
#define DEFAULT_CHUNK_SIZE  65536 /* 64 KB */

const struct option long_options[] = {
    {"file",           required_argument, NULL, 'f'},
    {"size",           required_argument, NULL, 's'},
    {"fixed-ratio",    required_argument, NULL, 'r'},
    {"chunk-size",     required_argument, NULL, 'S'},
    {"chunk-size-max", required_argument, NULL, 'M'},
    {"chunk-size-min", required_argument, NULL, 'm'},
    {"quiet",          no_argument,       NULL, 'q'},
    {"gen-holes",      no_argument,       NULL, 'H'},
    {"holes-size",     required_argument, NULL, 'O'},
    {"num-holes",      required_argument, NULL, 'N'},
    {"help",           no_argument,       NULL, 'h'},
};
const static char *short_options = "f:s:r:S:M:m:qHO:N:h";

static param_t g_param = {
    .filename               = NULL,
    .filesize               = 0,
    .fixed_ratio            = DEFAULT_FIXED_RATIO,
    .non_fixed_ratio        = 100 - DEFAULT_FIXED_RATIO,
    .fixed_part_size        = 0,
    .non_fixed_part_size    = 0,
    .chunk_size             = DEFAULT_CHUNK_SIZE,
    .chunk_size_min         = DEFAULT_CHUNK_SIZE,
    .chunk_size_max         = DEFAULT_CHUNK_SIZE,
    .quiet                  = 0,
    .enable_holes           = 0,
    .num_holes              = 0,
    .holes_size             = 0,
};

static void print_usage(const char *progname)
{
    const char *usage = 
    "Name:\n"
    "   %s - A dummy file generator for determining the efficiency of the file dedupplication\n"
    "\n"
    "Usage:\n"
    "   %s -f <filename> -s <size> [OPTION]..."
    "   This generator will try generating a file with given name <filename> and given size <size>\n"
    "\n"
    "[REQUIRED]:\n"
    "    -f, --file                specify the filename of the generating file\n"
    "    -s, --size                specify the size of the generating file\n"
    "                              support unit = { B, KB, MB, GB }\n"
    "                              for example: \"-s 100MB\" will generate a file with size 100MB\n"
    "\n"
    "[OPTION]:\n"
    "general:\n"
    "    -r, --fixed-ratio         specify the fixed part ratio of this generating file\n"
    "                              support range = [ 0 - 100 ], only integer input is available\n"
    "                              for example \"-r 20\" means that there will be 20%% fixed parts\n"
    "                              inside the generating file\n"
    "\n"
    "chunks:\n"
    "    support unit for chunk size = { B, KB, MB }\n"
    "\n"
    "    -S, --chunk-size          specify the size of each fixed chunks\n"
    "                              default chunk size = 65536 bytes ( 64 KB )\n"
    "    -m, --chunk-size-min      specify the minimal size of the varient-length generating chunks\n"
    "    -M, --chunk-size-max      specify the maximal size of the varient-length generating chunks\n"
    "\n"
    "holes:\n"
    "    -H, --gen-holes           allow generating holes in the generating file\n"
    "    -O, --holes-size          specify the total size of the holes in the generating file\n"
    "    -N, --num-holes           specify the total number of the holes in generating file\n"
    "\n"
    "others:\n"
    "    -q, --quiet               enable silent mode\n"
    "    -h, --help                display this help text\n"
    "\n"
    "Examples:\n"
    "\n"
    "Notes:\n"
    "\n";

    fprintf(stdout, usage, progname, progname);
}

static void print_info(void)
{
    char *fsize_str = bytes_to_unit(g_param.filesize, UNIT_FORMAT_NORMAL);
    char *fixed_part_size_str = bytes_to_unit(g_param.fixed_part_size, UNIT_FORMAT_NORMAL);
    char *non_fixed_part_size_str = bytes_to_unit(g_param.non_fixed_part_size, UNIT_FORMAT_NORMAL);
    char *chunksize_str = bytes_to_unit(g_param.chunk_size, UNIT_FORMAT_BYTES_ONLY);
    char *chunksize_min_str = bytes_to_unit(g_param.chunk_size_min, UNIT_FORMAT_BYTES_ONLY);
    char *chunksize_max_str = bytes_to_unit(g_param.chunk_size_max, UNIT_FORMAT_BYTES_ONLY);
    char *total_holes_size_str = bytes_to_unit(g_param.holes_size, UNIT_FORMAT_BYTES_ONLY);

    const char *info = ""
    "------------------------------------------------------------------------\n"
    "|                        [ Parameters Setting ]                        |\n"
    "|----------------------------------------------------------------------|\n"
    "|[General]                                                             |\n"
    "|    filename:            %-45s|\n"
    "|    filesize:            %-45s|\n"
    "|    fixed ratio:         %-3d %%                                        |\n"
    "|    fixed part size:     %-45s|\n"
    "|    non fixed part size: %-45s|\n"
    "|                                                                      |\n"
    "|[Chunk]                                                               |\n"
    "|    chunk size:          %-24s                     |\n"
    "|    min chunk size:      %-24s                     |\n"
    "|    max chunk size:      %-24s                     |\n"
    "|                                                                      |\n"
    "|[Holes]                                                               |\n"
    "|    generate holes :     %-44s |\n"
    "|    number of holes:     %-44lld |\n"
    "|    total size of holes: %-44s |\n"
    "|                                                                      |\n"
    "|[Others]                                                              |\n"
    "|                                                                      |\n"
    "------------------------------------------------------------------------\n"
    "";

    fprintf(stdout, info,
        g_param.filename,
        fsize_str,
        g_param.fixed_ratio,
        fixed_part_size_str,
        non_fixed_part_size_str,
        chunksize_str,
        chunksize_min_str,
        chunksize_max_str,
        g_param.enable_holes ? "enable" : "disable",
        g_param.num_holes,
        total_holes_size_str
        );

    free(fsize_str);
    free(fixed_part_size_str);
    free(non_fixed_part_size_str);
    free(chunksize_str);
    free(chunksize_min_str);
    free(chunksize_max_str);
    free(total_holes_size_str);
}

static int parse_cmds(int argc, char **argv)
{
    int opt = 0;

    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt)
        {
        case 'f':
            g_param.filename = strdup(optarg);
            break;
        case 's':
            g_param.filesize = unit_to_bytes(optarg);
            if (g_param.filesize <= 0) {
                fprintf(stderr, "filesize must be larger than 0 bytes!\n");
                return -1;
            }
            break;
        case 'r':
            g_param.fixed_ratio = atoi(optarg);
            if (g_param.fixed_ratio < 0 || g_param.fixed_ratio > 100) {
                fprintf(stderr, "fixed ratio should be a integer in range [ 0 - 100 ]\n");
                return -1;
            }
            g_param.non_fixed_ratio = 100 - g_param.fixed_ratio;
            break;
        case 'S':
            g_param.chunk_size = unit_to_bytes(optarg);
            if (g_param.chunk_size <= 0) {
                fprintf(stderr, "chunksize must be larget than 0 bytes\n");
                return -1;
            }
            break;
        case 'M':
            g_param.chunk_size_max = unit_to_bytes(optarg);
            if (g_param.chunk_size_max <= 0) {
                fprintf(stderr, "max chunk size must be larger than 0 bytes\n");
                return -1;
            }
            break;
        case 'm':
            g_param.chunk_size_min = unit_to_bytes(optarg);
            if (g_param.chunk_size_min <= 0) {
                fprintf(stderr, "min chunk size must be larger than 0 bytes\n");
                return -1;
            }
            break;
        case 'q':
            g_param.quiet = 1;
            break;
        case 'H':
            g_param.enable_holes = 1;
            break;
        case 'O':
            g_param.holes_size = unit_to_bytes(optarg);
            if (g_param.holes_size <= 0) {
                fprintf(stderr, "total size of the holes must be larger than 0 bytes\n");
                return -1;
            }
            break;
        case 'N':
            g_param.num_holes = atoi(optarg);
            if (g_param.num_holes <= 0) {
                fprintf(stderr, "total num of holes should be larger than 0\n");
                return -1;
            }
            break;
        case 'h':
        case '?':
        default:
            print_usage(argv[0]);
            break;
        }
    }

    return 0;
}

static int check_parameters(void)
{
    int error = 0;

    if (!g_param.filename) {
        fprintf(stderr, "[ERROR]: must set filename with -f <filename>\n");
        error++;
    }

    if (g_param.filesize <= 0) {
        fprintf(stderr, "[ERROR]: must set filesize >= 0 bytes with -s <size>\n");
        error++;
    }

    if (g_param.fixed_ratio < 0 || g_param.fixed_ratio > 100) {
        fprintf(stderr, "[ERROR]: fixed ratio must be a integer in range [ 0 - 100 ]\n");
        error++;
    }

    if (g_param.non_fixed_ratio < 0 || g_param.non_fixed_ratio > 100) {
        fprintf(stderr, "[ERROR]: invalid non fixed ratio\n");
        error++;
    }

    if (g_param.filesize < g_param.chunk_size) {
        fprintf(stderr, "[ERROR]: chunk size should always smaller than the file size\n");
        error++;
    }

    if (g_param.chunk_size_max < g_param.chunk_size_min) {
        fprintf(stderr, "[ERROR]: max chunk size should always greater or equal to min chunk size\n");
        error++;
    }

    if (g_param.enable_holes && g_param.holes_size <= 0) {
        fprintf(stderr, "[ERROR]: total size of the hole should always greater than 0 bytes\n");
        error++;
    }

    if (g_param.enable_holes && g_param.num_holes <= 0) {
        fprintf(stderr, "[ERROR]: number of holes should greater than 0 if enable generating holes mode\n");
        error++;
    }

    if (g_param.enable_holes && g_param.num_holes > g_param.holes_size) {
        fprintf(stderr, "[ERROR]: number of the holes should always smaller than the total size of the holes\n");
        error++;
    }

    return error;
}

static int prepare_generating_file(void)
{
    int64_t filesize = g_param.filesize;
    g_param.fixed_part_size     = filesize * g_param.fixed_ratio / 100;
    g_param.non_fixed_part_size = filesize - g_param.fixed_part_size;

    return 0;
}

int main(int argc, char **argv)
{
    /* used for generating random content */
    srand(time(NULL));

    if (parse_cmds(argc, argv)) {
        fprintf(stderr, "[WARN ]: Some errors occur when parsing commands\n");
        fprintf(stderr, "[WARN ]: Exiting the program...\n");
        return -1;
    }

    int num_err = 0;

    if ((num_err = check_parameters()) != 0) {
        fprintf(stderr, "[WARN ]: Total %d errors occur\n", num_err);
        fprintf(stderr, "[WARN ]: Exiting the program...\n");
        return -1;
    }

    if (prepare_generating_file()) {
        fprintf(stderr, "[WARN ]: Detect some invalid setting\n");
        fprintf(stderr, "[WARN ]: Exiting the program...\n");
        return -1;
    }

    if (!g_param.quiet)
        print_info();

    if (generate_file(&g_param)) {
        fprintf(stderr, "[WARN ]: Detect some error when generating file\n");
        fprintf(stderr, "[WARN ]: Exiting the program...\n");
        return -1;
    }

    return 0;
}