#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
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

    const char *info = ""
    "------------------------------------------------------------------------\n"
    "|========================[ Parameters Setting ]========================|\n"
    "|----------------------------------------------------------------------|\n"
    "|[General]                                                             |\n"
    "|    filename:    %-53s|\n"
    "|    filesize:    %-53s|\n"
    "|    fixed ratio: %-3d %%                                                |\n"
    "|                                                                      |\n"
    "|[Chunk]                                                               |\n"
    "|    chunk size:     %-18lld bytes                         |\n"
    "|    min chunk size: %-18lld bytes                         |\n"
    "|    max chunk size: %-18lld bytes                         |\n"
    "|                                                                      |\n"
    "|[Holes]                                                               |\n"
    "|    generate holes  :     %-44s|\n"
    "|    number of holes :     %-44lld|\n"
    "|    total size of holes : %-44lld|\n"
    "|                                                                      |\n"
    "|[Others]                                                              |\n"
    "|                                                                      |\n"
    "------------------------------------------------------------------------\n"
    "";

    fprintf(stdout, info,
        g_param.filename,
        fsize_str,
        g_param.fixed_ratio,
        g_param.chunk_size,
        g_param.chunk_size_min,
        g_param.chunk_size_max,
        g_param.enable_holes ? "enable" : "disable",
        g_param.num_holes,
        g_param.holes_size
        );

    free(fsize_str);
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

int main(int argc, char **argv)
{
    if (parse_cmds(argc, argv)) {
        fprintf(stderr, "Some errors occur when parsing commands\n");
        fprintf(stderr, "Exiting the program...\n");
        return -1;
    }

    if (!g_param.quiet)
        print_info();

    return 0;
}