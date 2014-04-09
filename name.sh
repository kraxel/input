#!/bin/sh

TYPE="$1"
INPUT="/usr/include/linux/input.h"

awk "
	/KEY_MIN_INTERESTING/	{next};
	/EV_VERSION/	{ next };
	/_MAX/		{ next };
	/_CNT/		{ next };
	/#define $1_/	{ 
		name = \$2;
                if (\"$TYPE\" != \"BTN\") {
			sub(\"^${TYPE}_\", \"\", name);
		}
		printf(\"\t[ %-16s ] = \\\"%s\\\",\n\", \$2, name);
	}
" < $INPUT
