# config
-include Make.config
include mk/Variables.mk

# add our flags + libs
CFLAGS	+= -DVERSION='"$(VERSION)"'
LDLIBS	+= -lm

# build
TARGETS	:= lsinput input-events input-kbd input-send input-recv lircd.conf
HEADERS	:= EV.h REL.h KEY.h BTN.h BUS.h

# default target
all: build


#################################################################
# poor man's autoconf ;-)

include mk/Autoconf.mk

define make-config
LIB		:= $(LIB)
endef


########################################################################
# rules

build: $(TARGETS)

$(HEADERS): name.sh
	sh name.sh $* > $@

lircd.conf: lirc.sh 
	sh lirc.sh > $@

lsinput: lsinput.o input.o
input-events: input-events.o input.o
input-kbd: input-kbd.o input.o
input-send: input-send.o input.o tcp.o
input-recv: input-recv.o input.o tcp.o

input.o: input.c $(HEADERS)

install: build
	install -d $(bindir)
	install -s lsinput input-events input-kbd input-send input-recv $(bindir)

clean:
	-rm -f *.o $(depfiles)

realclean distclean: clean
	-rm -f Make.config
	-rm -f $(TARGETS) $(HEADERS) *~ xpm/*~ *.bak

#############################################

include mk/Compile.mk
include mk/Maintainer.mk
-include $(depfiles)

