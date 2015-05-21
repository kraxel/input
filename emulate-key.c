#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include "input.h"

/* ------------------------------------------------------------------ */

static int usage(char *prog, int error)
{
	fprintf(error ? stderr : stdout,
		"usage: %s -h            this text\n"
		"usage: %s -l            list keys\n"
		"usage: %s [ -v ] <key>  send key\n",
		prog, prog, prog);
	exit(error);
}

static int send_key(int fd, int key, bool down)
{
        struct input_event ev;

        memset(&ev, 0, sizeof(ev));
        ev.type = EV_KEY;
        ev.code = key;
        ev.value = down ? 0 : 1;
        write(fd, &ev, sizeof(ev));

        memset(&ev, 0, sizeof(ev));
        ev.type = EV_SYN;
        write(fd, &ev, sizeof(ev));

        return 0;
}

int main(int argc, char *argv[])
{
        static struct uinput_user_dev uidev = {
                .name = "emulate-key",
        };
        int verbose = 0;
	int c, key, fd;
        char *name;

	for (;;) {
		if (-1 == (c = getopt(argc, argv, "hvl")))
			break;
		switch (c) {
                case 'v':
                        verbose = 1;
                        break;
                case 'l':
                        for (key = 0; key < KEY_MAX; key++) {
                                name = ev_type_name(EV_KEY, key);
                                if (!name)
                                        continue;
                                if (strcmp(name, "???") == 0)
                                        continue;
                                printf("%s%s", key ? ", " : "", name);
                        }
                        printf("\n");
                        exit(0);
		case 'h':
			usage(argv[0],0);
		default:
			usage(argv[0],1);
		}
	}

        if (optind == argc) {
                fprintf(stderr, "%s: no keyname given\n",
                        argv[0]);
                exit(1);
        }
        for (key = 0; key < KEY_MAX; key++) {
                if (strcmp(ev_type_name(EV_KEY, key), argv[optind]) == 0) {
                        break;
                }
        }
        if (key == KEY_MAX) {
                fprintf(stderr, "%s: key '%s' not found\n",
                        argv[0], argv[optind]);
                exit(1);
        }
        if (verbose) {
                printf("Sending '%s' (%d) key (down+up).\n", argv[optind], key);
        }

        fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        if (fd < 0) {
                perror("open /dev/uinput");
                exit(1);
        }

        ioctl(fd, UI_SET_EVBIT, EV_SYN);
        ioctl(fd, UI_SET_EVBIT, EV_KEY);
        ioctl(fd, UI_SET_KEYBIT, key);
        write(fd, &uidev, sizeof(uidev));
        ioctl(fd, UI_DEV_CREATE);

        usleep(100000);
        send_key(fd, key, true);
        usleep(100000);
        send_key(fd, key, false);
        usleep(100000);

        ioctl(fd, UI_DEV_DESTROY);

	return 0;
}

/* ---------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
