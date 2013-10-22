SRC_INC=-I/mfs-gcc/include -Iinclude -I/usr/include -I.
#CFLAGS=-Os -w -march=i386 -fno-strict-aliasing -funsigned-char -Wundef -Wno-char-subscripts -Wimplicit -Wall -pedantic
CFLAGS=-Os -w -fno-strict-aliasing -funsigned-char -Wundef -Wno-char-subscripts -Wimplicit -Wall -pedantic
LDLIBS=-lsqlite -lreadline -lcurses -rdynamic -ldl -lpcap -L/mfs-gcc/lib -ldnet
CC=gcc

all: convert clean mybox

convert:
	@dos2unix lib/*.c
	@dos2unix include/*.h
	@dos2unix prog/*.c
	@dos2unix makefile

mybox:
	${CC} ${CFLAGS} ${SRC_INC} -c lib/*.c prog/*.c
	${CC} ${CFLAGS} ${SRC_INC} ${LDLIBS} mybox.c -o mybox *.o 
	@strip mybox

clean distclean:
	rm -f mybox *.o
