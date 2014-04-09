#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>

#include "input.h"

/* ------------------------------------------------------------------ */

char *EV_NAME[EV_CNT] = {
#include "EV.h"
};

static char *REL_NAME[REL_CNT] = {
#include "REL.h"
};

static char *ABS_NAME[ABS_CNT] = {
#include "ABS.h"
};

static char *MSC_NAME[MSC_CNT] = {
#include "MSC.h"
};

static char *LED_NAME[LED_CNT] = {
#include "LED.h"
};

static char *SND_NAME[SND_CNT] = {
#include "SND.h"
};

#ifndef REP_CNT
#define REP_CNT (REP_MAX + 1)
#endif

static char *REP_NAME[REP_CNT] = {
#include "REP.h"
};

static char *KEY_NAME[KEY_CNT] = {
#include "KEY.h"
#include "BTN.h"
};

static char *SW_NAME[SW_CNT] = {
#include "SW.h"
};

int EV_TYPE_MAX[EV_CNT] = {
	[EV_REL] = REL_MAX,
	[EV_ABS] = ABS_MAX,
	[EV_MSC] = MSC_MAX,
	[EV_LED] = LED_MAX,
	[EV_SND] = SND_MAX,
	[EV_REP] = REP_MAX,
	[EV_KEY] = KEY_MAX,
	[EV_SW] = SW_MAX,
};

char **EV_TYPE_NAME[EV_CNT] = {
	[EV_REL] = REL_NAME,
	[EV_ABS] = ABS_NAME,
	[EV_MSC] = MSC_NAME,
	[EV_LED] = LED_NAME,
	[EV_SND] = SND_NAME,
	[EV_REP] = REP_NAME,
	[EV_KEY] = KEY_NAME,
	[EV_SW] = SW_NAME,
};

char *BUS_NAME[] = {
#include "BUS.h"
};

/* ------------------------------------------------------------------ */

int device_open(int nr, bool verbose)
{
	char filename[32];
	int fd;

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

	return fd;
}

int device_info(int nr, int fd, bool verbose)
{
	struct input_id id;
	char name[64];
	char phys[64];
	char buf[64];
	BITFIELD evtmap[32];
        BITFIELD bitmap[256];
	int rc, pos, evts, evt, bits, bit, count;

	rc = ioctl(fd,EVIOCGID,&id);
	if (rc < 0)
                return -1;

        memset(name, 0, sizeof(name));
        memset(phys, 0, sizeof(phys));
        memset(evtmap, 0, sizeof(evtmap));

	ioctl(fd, EVIOCGNAME(sizeof(name)-1), name);
	ioctl(fd, EVIOCGPHYS(sizeof(phys)-1), phys);
	evts = ioctl(fd, EVIOCGBIT(0,sizeof(evtmap)), evtmap);

        if (!verbose) {
                memset(buf, 0, sizeof(buf));
                for (evt = 0, pos = 0;
                     evt < evts*8 && evt < EV_MAX;
                     evt++) {
                        if (evt == EV_SYN || evt == EV_REP)
                                continue;
                        if (!test_bit(evt,evtmap))
                                continue;
                        if (pos >= sizeof(buf))
                                continue;
                        pos += snprintf(buf+pos, sizeof(buf)-pos, "%s%s",
                                        pos ? " " : "",
                                        EV_NAME[evt]);
                }
                fprintf(stderr, "%2d: %04x:%04x %-6.6s %-16.16s %-24.24s %-16s\n", nr,
                        id.vendor, id.product, BUS_NAME[id.bustype],
                        phys, name, buf);
        } else {
                fprintf(stderr, "   id   : %04x:%04x, %s, v%d\n",
                        id.vendor, id.product, BUS_NAME[id.bustype], id.version);
                fprintf(stderr, "   phys : \"%s\"\n",phys);
                fprintf(stderr, "   name : \"%s\"\n",name);

                for (evt = 0; evt < evts*8 && evt < EV_MAX; evt++) {
                        if (evt == EV_SYN || evt == EV_REP)
                                continue;
                        if (!test_bit(evt,evtmap))
                                continue;
                        bits = ioctl(fd, EVIOCGBIT(evt,sizeof(bitmap)), bitmap);
                        memset(buf, 0, sizeof(buf));
                        for (bit = 0, count = 0, pos = 0;
                             bit < bits*8 && bit < EV_TYPE_MAX[evt];
                             bit++) {
                                if (!test_bit(bit,bitmap))
                                        continue;
                                count++;
                                if (pos >= sizeof(buf))
                                        continue;
                                pos += snprintf(buf+pos, sizeof(buf)-pos, "%s%s",
                                                pos ? " " : "",
                                                EV_TYPE_NAME[evt][bit]);
                        }
                        if (pos >= sizeof(buf)) {
                                fprintf(stderr, "   %-4.4s : [ %d codes ]\n",
                                        EV_NAME[evt], count);
                        } else {
                                fprintf(stderr, "   %-4.4s : %s\n",
                                        EV_NAME[evt], buf);
                        }
                }

                fprintf(stderr,"\n");
        }

        return 0;
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
			ev_type_name(EV_KEY, event->code),
			event->value ? "pressed" : "released");
		break;
	case EV_REL:
	case EV_ABS:
	case EV_MSC:
	case EV_LED:
	case EV_SND:
	case EV_REP:
	case EV_SW:
		fprintf(stderr," %s %d",
			ev_type_name(event->type, event->code), event->value);
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
