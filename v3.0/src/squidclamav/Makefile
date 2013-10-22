# Makefile for cfav

OFILES = cfav.o main.o config.o log.o lists.o

HFILES = cfav.h paths.h lists.h log.h

CC=gcc

#EXTRALIBS=-lcurl -ldl -lcrypto -ldl -lz -lbz2 -lgmp -lpthread
EXTRALIBS=-lcurl

BINARIES=cfav

#CFLAGS ?= -O3 -Wall -funroll-loops -fno-builtin-log
CFLAGS=-Os -w -march=i386 -fno-strict-aliasing -funsigned-char -Wundef -Wno-char-subscripts -Wimplicit -Wall -pedantic

all: $(BINARIES)
			
cfav.o: cfav.c $(HFILES)
	$(CC) -c cfav.c $(CFLAGS)

main.o: main.c $(HFILES)
	$(CC) -c main.c $(CFLAGS)

config.o: config.c $(HFILES)
	$(CC) -c config.c $(CFLAGS)

log.o: log.c $(HFILES)
	$(CC) -c log.c $(CFLAGS)

lists.o: lists.c $(HFILES)
	$(CC) -c lists.c $(CFLAGS)

cfav: $(OFILES) $(HFILES)
	$(CC) -o cff $(OFILES)	$(EXTRALIBS)

clean:
	@rm -f *.o cfav cff


