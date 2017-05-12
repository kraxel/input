#!/bin/sh

OUT="${1-lircd.conf}"

if test -f "/usr/include/linux/input-event-codes.h"
then
	INPUT="/usr/include/linux/input-event-codes.h"
else
	INPUT="/usr/include/linux/input.h"
fi
cat <<EOF > "$OUT"
begin remote
	name linux-input-layer
	bits 32
	begin codes
EOF
awk "
	/_MAX/			{ next };
	/KEY_RESERVED/		{ next };
	/#define (KEY|BTN)_/	{ gsub(/KEY_/,\"\",\$2);
				  printf(\"\t\t%-20s 0x%04x\n\",
					 \$2,0x10000+strtonum(\$3)) } 
" < $INPUT >> "$OUT"
cat <<EOF  >> "$OUT"
	end codes
end remote
EOF
