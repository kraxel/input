#include <inttypes.h>

#include <sys/ioctl.h>
#include "linux-input.h"

#define ev_name(code)  ((code) < EV_MAX  && EV_NAME[code]  ? EV_NAME[code]  : "???")
#define rel_name(code) ((code) < REL_MAX && REL_NAME[code] ? REL_NAME[code] : "???")
#define key_name(code) ((code) < KEY_MAX && KEY_NAME[code] ? KEY_NAME[code] : "???")

#define BITFIELD uint32_t

extern char *EV_NAME[EV_MAX];
extern char *REL_NAME[REL_MAX];
extern char *KEY_NAME[KEY_MAX];
extern char *BUS_NAME[];

static __inline__ int test_bit(int nr, BITFIELD * addr)
{
	BITFIELD mask;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	return ((mask & *addr) != 0);
}

/* ------------------------------------------------------------------ */

int device_open(int nr, int verbose);
void device_info(int fd);
void print_event(struct input_event *event);
