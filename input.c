#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>

#include "input.h"

/* ------------------------------------------------------------------ */

char *EV_NAME[EV_MAX] = {
#include "EV.h"
};

char *REL_NAME[REL_MAX] = {
#include "REL.h"
};

char *KEY_NAME[KEY_MAX] = {
#include "KEY.h"
#include "BTN.h"
};

char *BUS_NAME[] = {
#include "BUS.h"
};

/* ------------------------------------------------------------------ */

int device_open(int nr, int verbose)
{
	char filename[32];
	int fd, version;

	snprintf(filename,sizeof(filename),
		 "/dev/input/event%d",nr);
	fd = open(filename,O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr,"open %s: %s\n",
			filename,strerror(errno));
		return -1;
	}
	if (verbose)
		fprintf(stderr,"%s\n",filename);

	if (-1 == ioctl(fd,EVIOCGVERSION,&version)) {
		perror("ioctl EVIOCGVERSION");
		close(fd);
		return -1;
	}
	if (EV_VERSION != version) {
		fprintf(stderr, "protocol version mismatch (expected %d, got %d)\n",
			EV_VERSION, version);
		close(fd);
		return -1;
	}
	return fd;
}

void device_info(int fd)
{
	struct input_id id;
	BITFIELD bits[32];
	char buf[32];
	int rc,bit;

	rc = ioctl(fd,EVIOCGID,&id);
	if (rc >= 0)
		fprintf(stderr,
			"   bustype : %s\n"
			"   vendor  : 0x%x\n"
			"   product : 0x%x\n"
			"   version : %d\n",
			BUS_NAME[id.bustype],
			id.vendor, id.product, id.version);
	rc = ioctl(fd,EVIOCGNAME(sizeof(buf)),buf);
	if (rc >= 0)
		fprintf(stderr,"   name    : \"%.*s\"\n",rc,buf);
	rc = ioctl(fd,EVIOCGPHYS(sizeof(buf)),buf);
	if (rc >= 0)
		fprintf(stderr,"   phys    : \"%.*s\"\n",rc,buf);
	rc = ioctl(fd,EVIOCGUNIQ(sizeof(buf)),buf);
	if (rc >= 0)
		fprintf(stderr,"   uniq    : \"%.*s\"\n",rc,buf);
	rc = ioctl(fd,EVIOCGBIT(0,sizeof(bits)),bits);
	if (rc >= 0) {
		fprintf(stderr,"   bits ev :");
		for (bit = 0; bit < rc*8 && bit < EV_MAX; bit++) {
			if (test_bit(bit,bits))
				fprintf(stderr," %s", EV_NAME[bit]);
		}
		fprintf(stderr,"\n");
	}
	fprintf(stderr,"\n");
}

/* ------------------------------------------------------------------ */

void print_event(struct input_event *event)
{
	char ts[32];
	time_t t;

	t = event->time.tv_sec;
	strftime(ts,sizeof(ts),"%H:%M:%S",localtime(&t));
	fprintf(stderr,"%s.%06ld: %s",ts,event->time.tv_usec,
		EV_NAME[event->type]);
	switch (event->type) {
	case EV_KEY:
		fprintf(stderr," %s %s",
			KEY_NAME[event->code],
			event->value ? "pressed" : "released");
		break;
	case EV_REL:
		fprintf(stderr," %s %d",
			REL_NAME[event->code], event->value);
		break;
	default:
		fprintf(stderr," code=%u value=%d",
			(unsigned int)event->code, event->value);
	}
	fprintf(stderr,"\n");
}

/* ---------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
