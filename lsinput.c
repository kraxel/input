#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "input.h"

/* ------------------------------------------------------------------ */

static void list_devices(void)
{
	int i,fd;
        char filename[32];
        struct stat statbuf;

	for (i = 0; i < 32; i++) {
                snprintf(filename,sizeof(filename), "/dev/input/event%d",i);
                if (stat(filename, &statbuf) == 0) {
                        /* try to open */
                        fd = device_open(i,1);
                        if (-1 == fd)
                                return;
                        device_info(fd);
                        close(fd);
                }
	}
	return;
}

int main(int argc, char *argv[])
{
	list_devices();
	exit(0);
}

/* ---------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
