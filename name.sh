#!/bin/sh

TYPE="$1"
if test -f "/usr/include/linux/input-event-codes.h" -a "$TYPE" != "BUS"; then
	INPUT="/usr/include/linux/input-event-codes.h"
else
	INPUT="/usr/include/linux/input.h"
fi

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
" < $INPUT > "${TYPE}.h"
