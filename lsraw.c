#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <linux/hidraw.h>

/* ------------------------------------------------------------------ */

static const char *tag_map[4][16] = {
    {
        /* main */
        [  8 ] = "input",
        [  9 ] = "output",
        [ 11 ] = "feature",
        [ 10 ] = "collection",
        [ 12 ] = "end",
    },{
        /* global */
        [  0 ] = "usage page",
        [  1 ] = "logical min",
        [  2 ] = "logical max",
        [  3 ] = "physical min",
        [  4 ] = "physical max",
        [  5 ] = "unit exponent",
        [  6 ] = "unit",
        [  7 ] = "report size",
        [  8 ] = "report id",
        [  9 ] = "report count",
        [ 10 ] = "push",
        [ 11 ] = "pop",
    },{
        /* local */
        [  0 ] = "usage",
        [  1 ] = "usage min",
        [  2 ] = "usage max",
        [  3 ] = "desig index",
        [  4 ] = "desig min",
        [  5 ] = "desig max",
        [  7 ] = "string index",
        [  8 ] = "string min",
        [  9 ] = "string max",
        [ 10 ] = "delimiter",
    },{
        /* reserved */
    }
};
static const char *type_map[4] = {
    "main", "global", "local", "reserved"
};
static const int size_map[4] = {
    0, 1, 2, 4
};

static void print_report(unsigned char *desc, int descsize)
{
    unsigned data;
    int tag, type, size, bytes;
    int i, off = 0;

    while (off < descsize) {
        if (desc[off] == 0xfe) {
            printf("TODO: long item\n");
            return;
        } else {
            tag  = (desc[off] >> 4) & 0x0f;
            type = (desc[off] >> 2) & 0x03;
            size = (desc[off] >> 0) & 0x03;
            printf("  %-16s %-6s %d  --",
                   tag_map[type][tag] ?: "?",
                   type_map[type],
                   size_map[size]);

            data = 0;
            for (i = 0; i < size_map[size]; i++)
                data |= desc[off + 1 + i] << (8 * i);
            printf("  %8x  -- ", data);

            bytes = size_map[size] + 1;
            for (i = 0; i < bytes && off + i < descsize; i++)
                printf(" %02x", desc[off+i]);
            printf("\n");

            off += bytes;
        }
    }
}

static int usage(char *prog, int error)
{
    fprintf(error ? stderr : stdout,
            "usage: %s"
            " [ -h ]\n",
            prog);
    exit(error);
}

int main(int argc, char *argv[])
{
    struct hidraw_report_descriptor desc;
    int i, c, fd, rc, size;
    char devname[32];

    for (;;) {
        if (-1 == (c = getopt(argc, argv, "hv")))
            break;
        switch (c) {
        case 'h':
            usage(argv[0], 0);
        default:
            usage(argv[0], 1);
        }
    }

    for (i = 0; i < HIDRAW_MAX_DEVICES; i++) {
        snprintf(devname, sizeof(devname), "/dev/hidraw%d", i);
        fd = open(devname, O_RDWR);
        if (fd < 0) {
            if (errno != ENOENT) {
                fprintf(stderr, "%s: %s\n", devname, strerror(errno));
                continue;
            } else {
                break;
            }
        }

        rc = ioctl(fd, HIDIOCGRDESCSIZE, &size);

        desc.size = size;
        rc = ioctl(fd, HIDIOCGRDESC, &desc);
        if (rc < 0) {
            fprintf(stderr, "%s: HIDIOCGRDESC: %s\n", devname, strerror(errno));
            continue;
        }
        close(fd);

        printf("%s (%d)\n", devname, size);
        print_report(desc.value, size);
    }

    return 0;
}
