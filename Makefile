CC=gcc
CFLAGS= -g -funroll-loops -Wall -mms-bitfields 
#LDFLAGS= -lefence
#LDFLAGS=`pkg-config --libs gtk+-2.0`
SRCMAIN=genpathmaps.c 
OBJMAIN=genpathmaps.o
COMMONHDR=common.h
SRCFILES=pathfindingmap.c smallones.c image.c memory.c textfile.c commonutils.c
OBJFILES=pathfindingmap.o smallones.o image.o memory.o textfile.o commonutils.o
HDRFILES=pathfindingmap.h smallones.h image.h memory.h textfile.h commonutils.h
TARFILES=$(SRCFILES) $(DESTFILE) Makefile
DESTFILE=genpathmaps

all: genpathmaps

install: genpathmaps
	install genpathmaps /usr/bin

tar: genpathmaps.tar.gz

genpathmaps.tar.gz: Makefile $(SRCFILES) $(DESTFILE)
	-rm -f $(DESTFILE).tar.bz2
	tar cvjf $(DESTFILE).tar.bz2 $(TARFILES)

genpathmaps: $(OBJMAIN) $(OBJFILES)
	$(CC) -o $@ $(OBJMAIN) $(OBJFILES) $(LDFLAGS)

genpathmaps.o: $(SRCMAIN) $(COMMONHDR) $(HDRFILES)
	$(CC) $(CFLAGS) -c $(SRCMAIN)

%.o: %.c $(COMMONHDR) $(HDRFILES)
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f $(OBJFILES) $(DESTFILE) $(OBJMAIN) core core.* *.dump
