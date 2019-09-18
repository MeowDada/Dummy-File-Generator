#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

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

static int parse_cmds(int argc, char **argv)
{
    int opt = 0;

    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL) != -1)) {
        switch (opt)
        {
        case 'f':
            break;
        case 's':
            break;
        case 'S':
            break;
        case 'M':
            break;
        case 'm':
            break;
        case 'q':
            break;
        case 'H':
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

    return 0;
}