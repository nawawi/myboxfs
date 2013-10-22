SRC_INC=-I/mfs-gcc/include -Iinclude -I/usr/include -I.
CFLAGS=-w -march=i386 -fno-strict-aliasing -funsigned-char -Wundef -Wno-char-subscripts -Wimplicit -Wall -pedantic
LDLIBS=-lsqlite -lreadline -lcurses -rdynamic -ldl -lpcap -L/mfs-gcc/lib -ldnet -lcrypt
CC=gcc

all: convert clean mybox trafficd vmware sophos

convert:
	@dos2unix lib/*.c
	@dos2unix include/*.h
	@dos2unix prog/*.c
	@dos2unix makefile
	@dos2unix *.c

mybox:
	${CC} ${CFLAGS} ${SRC_INC} -c lib/*.c prog/*.c
	${CC} ${CFLAGS} ${SRC_INC} ${LDLIBS} mybox.c -o mybox *.o 
	@strip mybox

trafficd:
	${CC} ${CFLAGS} -I../libipt_ACCOUNT-1.2/src -L../libipt_ACCOUNT-1.2/src/.libs trafficd.c -o trafficd -lipt_ACCOUNT lib.o
	@strip trafficd

vmware:
	${CC} ${CFLAGS} -o vmware-detect vmware-detect.c

sophos:
	${CC} ${CFLAGS} -o sophos_scan sophos_scan.c
	#${CC} ${CFLAGS} -Wl,-rpath,/usr/local/lib -L/usr/local/lib -lsavi -Iinclude/sav -o sophos_version sophos_version.c

clean distclean:
	rm -f mybox trafficd vmware-detect sophos_scan sophos_version *.o
