#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>

#include "input.h"

struct kbd_entry {
	int scancode;
	int keycode;
};

struct kbd_map {
	struct kbd_entry  *map;
	int               keys;
	int               size;
	int               alloc;
};

/* ------------------------------------------------------------------ */

static struct kbd_map* kbd_map_read(int fd)
{
	struct kbd_entry entry;
	struct kbd_map *map;
	int rc;

	map = malloc(sizeof(*map));
	memset(map,0,sizeof(*map));
	for (map->size = 0; map->size < 65536; map->size++) {
		entry.scancode = map->size;
		entry.keycode  = KEY_RESERVED;
		rc = ioctl(fd, EVIOCGKEYCODE, &entry);
		if (rc < 0) {
			map->size--;
			break;
		}
		if (map->size >= map->alloc) {
			map->alloc += 64;
			map->map = realloc(map->map, map->alloc * sizeof(entry));
		}
		map->map[map->size] = entry;

		if (KEY_RESERVED != entry.keycode)
			map->keys++;
	}
	if (map->keys) {
		fprintf(stderr,"map: %d keys, size: %d/%d\n",
			map->keys, map->size, map->alloc);
		return map;
	} else {
		free(map);
		return NULL;
	}
}

static int kbd_map_write(int fh, struct kbd_map *map)
{
	int i,rc;

	for (i = 0; i < map->size; i++) {
		rc = ioctl(fh, EVIOCSKEYCODE, &map->map[i]);
		if (0 != rc) {
			fprintf(stderr,"ioctl EVIOCSKEYCODE(%d,%d): %s\n",
				map->map[i].scancode,map->map[i].keycode,
				strerror(errno));
			return -1;
		}
	}
	return 0;
}

static void kbd_key_print(FILE *fp, int scancode, int keycode)
{
	fprintf(fp, "0x%04x = %3d  # %s\n",
		scancode, keycode, key_name(keycode));
}

static void kbd_map_print(FILE *fp, struct kbd_map *map, int complete)
{
	int i;

	for (i = 0; i < map->size; i++) {
		if (!complete  &&  KEY_RESERVED == map->map[i].keycode)
			continue;
		kbd_key_print(fp,map->map[i].scancode,map->map[i].keycode);
	}
}

static int kbd_map_parse(FILE *fp, struct kbd_map *map)
{
	struct kbd_entry entry;
	char line[80],scancode[80],keycode[80];
	int i;

	while (NULL != fgets(line,sizeof(line),fp)) {
		if (2 != sscanf(line," %80s = %80s", scancode, keycode)) {
			fprintf(stderr,"parse error: %s",line);
			return -1;
		}

		/* parse scancode */
		if (0 == strncasecmp(scancode,"0x",2)) {
			entry.scancode = strtol(scancode, NULL, 16);
		} else {
			entry.scancode = strtol(scancode, NULL, 10);
		}
		if (entry.scancode <  0 ||
		    entry.scancode >= map->size) {
			fprintf(stderr,"scancode %d out of range (0-%d)\n",
				entry.scancode,map->size);
			return -1;
		}

		/* parse keycode */
		for (i = 0; i < KEY_MAX; i++) {
			if (!KEY_NAME[i])
				continue;
			if (0 == strcmp(keycode,KEY_NAME[i]))
				break;
		}
		if (i == KEY_MAX)
			entry.keycode = atoi(keycode);
		else
			entry.keycode = i;

		fprintf(stderr,"set: ");
		kbd_key_print(stderr,entry.scancode,entry.keycode);
		map->map[entry.scancode] = entry;
	}
	return 0;
}

/* ------------------------------------------------------------------ */

static void kbd_print_bits(int fd)
{
	BITFIELD bits[KEY_MAX/sizeof(BITFIELD)];
	int rc,bit;

	rc = ioctl(fd,EVIOCGBIT(EV_KEY,sizeof(bits)),bits);
	if (rc < 0)
		return;
	for (bit = 0; bit < rc*8 && bit < KEY_MAX; bit++) {
		if (!test_bit(bit,bits))
			continue;
		if (KEY_NAME[bit]) {
			fprintf(stderr,"bits: %s\n", KEY_NAME[bit]);
		} else {
			fprintf(stderr,"bits: unknown [%d]\n", bit);
		}
	}
}

static void show_kbd(int nr)
{
	struct kbd_map *map;
	int fd;

	fd = device_open(nr,1);
	if (-1 == fd)
		return;
	device_info(fd);

	map = kbd_map_read(fd);
	if (NULL != map) {
		kbd_map_print(stdout,map,0);
	} else {
		kbd_print_bits(fd);
	}

	close(fd);
}

static int set_kbd(int nr, char *mapfile)
{
	struct kbd_map *map;
	FILE *fp;
	int fd;

	fd = device_open(nr,1);
	if (-1 == fd)
		return -1;

	map = kbd_map_read(fd);
	if (NULL == map) {
		fprintf(stderr,"device has no map\n");
		close(fd);
		return -1;
	}

	if (0 == strcmp(mapfile,"-"))
		fp = stdin;
	else {
		fp = fopen(mapfile,"r");
		if (NULL == fp) {
			fprintf(stderr,"open %s: %s\n",mapfile,strerror(errno));
			close(fd);
			return -1;
		}
	}
	
	if (0 != kbd_map_parse(fp,map) ||
	    0 != kbd_map_write(fd,map)) {
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

static int usage(char *prog, int error)
{
	fprintf(error ? stderr : stdout,
		"usage: %s [ -f file ] devnr\n",
		prog);
	exit(error);
}

int main(int argc, char *argv[])
{
	int c,devnr;
	char *mapfile = NULL;

	for (;;) {
		if (-1 == (c = getopt(argc, argv, "hf:")))
			break;
		switch (c) {
		case 'f':
			mapfile = optarg;
			break;
		case 'h':
			usage(argv[0],0);
		default:
			usage(argv[0],1);
		}
	}

	if (optind == argc)
		usage(argv[0],1);

	devnr = atoi(argv[optind]);
	if (mapfile) {
		set_kbd(devnr,mapfile);
	} else {
		show_kbd(devnr);
	}
	return 0;
}

/* ---------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
