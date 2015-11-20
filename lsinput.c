#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "input.h"

/* ------------------------------------------------------------------ */

static void list_devices(int devnr, int verbose)
{
	int i,fd;
        char filename[32];
        struct stat statbuf;

	for (i = 0; i < 32; i++) {
                if (devnr > 0 && devnr != i)
                        continue;
                snprintf(filename,sizeof(filename), "/dev/input/event%d",i);
                if (stat(filename, &statbuf) != 0)
                        continue;
                fd = device_open(i, verbose);
                if (-1 == fd)
                        continue;
                device_info(i, fd, verbose);
                close(fd);
	}
	return;
}

static int usage(char *prog, int error)
{
	fprintf(error ? stderr : stdout,
		"usage: %s"
		" [ -v ] [ -s <devnr> ]\n",
		prog);
	exit(error);
}

int main(int argc, char *argv[])
{
        int verbose = 0;
        int devnr = -1;
        int c;

	for (;;) {
		if (-1 == (c = getopt(argc, argv, "hvs:")))
			break;
		switch (c) {
		case 's':
			devnr = atoi(optarg);
			break;
		case 'v':
			verbose++;
			break;
		case 'h':
			usage(argv[0],0);
		default:
			usage(argv[0],1);
		}
	}

	list_devices(devnr, verbose);
	exit(0);
}

/* ---------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
