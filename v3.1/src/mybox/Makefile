_TOP=/root/mybox-build/src
SRC_INC=-I${_TOP}/kernel/kernel/include -I./include -I/usr/include
CLISH_INC=-I${_TOP}/clish/inst/include
CLISH_LIB=-L ${_TOP}/clish/inst/lib -lclish -ltinyxml -llub -ltinyrl -lpthread
CFLAGS=-w -fno-strict-aliasing -funsigned-char -Wundef -Wno-char-subscripts -Wimplicit -Wall -pedantic
LDLIBS=-ldl -lcrypt
CC=gcc

_LDIR=./lib
_SDIR=./src
_OBJC=	lib/xlib.c lib/get_boottime.c lib/print_banner.c \
	lib/init_lang.c lib/read_oneline.c \
	lib/xmkdir.c lib/stristr.c lib/xsystem.c \
	lib/file_exists.c lib/trim.c lib/stripchar.c \
	lib/split_array.c lib/save_append_to_file.c lib/splitc.c \
	lib/is_dir.c lib/substr.c lib/str_replace.c

all: convert clean obj shell prog libmybox

convert:
	@dos2unix lib/*.c
	@dos2unix include/*.h
	@dos2unix src/*.c
	@dos2unix Makefile

obj:
	${CC} ${CFLAGS} ${SRC_INC} -c ${_OBJC}
	ar crs liblogin.a \
		xlib.o substr.o print_banner.o xsystem.o init_lang.o \
                file_exists.o str_replace.o read_oneline.o trim.o 

libmybox:
	ar rcs libmybox.a *.o
	gcc -shared -Wl,-soname,libmybox.so -o libmybox.so.1 *.o ${CLISH_INC} ${CLISH_LIB}

iosh.o:
	${CC} ${CFLAGS} ${SRC_INC} -c ${_LDIR}/iosh.c -o iosh.o ${CLISH_INC} ${CLISH_LIB}

login.o:
	${CC} ${CFLAGS} ${SRC_INC} -c ${_LDIR}/login.c -o login.o


shell: iosh.o login.o
	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/login ${_SDIR}/login.c \
		login.o iosh.o ${CLISH_INC} ${CLISH_LIB} ${LDLIBS} \
		liblogin.a	

	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/iosh ${_SDIR}/iosh.c \
		login.o iosh.o ${CLISH_INC} ${CLISH_LIB} ${LDLIBS} \
		liblogin.a

prog:
	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/boottime ${_SDIR}/boottime.c get_boottime.o
	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/chkprog ${_SDIR}/chkprog.c xlib.o \
		read_oneline.o xmkdir.o xsystem.o file_exists.o trim.o save_append_to_file.o \
		splitc.o

	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/cpu ${_SDIR}/cpu.c xlib.o
	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/getkey ${_SDIR}/getkey.c
	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/lcdd ${_SDIR}/lcdd.c xlib.o \
		file_exists.o trim.o stripchar.o split_array.o save_append_to_file.o

	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/loadavg ${_SDIR}/loadavg.c xlib.o

	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/trafficd ${_SDIR}/trafficd.c xlib.o \
		-I${_TOP}/libipt_ACCOUNT/inst/include -L${_TOP}/libipt_ACCOUNT/inst/lib -lipt_ACCOUNT \
		xmkdir.o file_exists.o save_append_to_file.o is_dir.o

	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/uplinkd ${_SDIR}/uplinkd.c xlib.o \
		read_oneline.o xsystem.o file_exists.o trim.o

	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/banner ${_SDIR}/banner.c print_banner.o
	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/vmware-detect ${_SDIR}/vmware-detect.c
	${CC} ${CFLAGS} ${SRC_INC} -o ${_SDIR}/sophos_scan ${_SDIR}/sophos_scan.c
	${CC} ${CFLAGS} -o ${_SDIR}/syslog-filter ${_SDIR}/syslog-filter.c \
		xlib.o file_exists.o

	${CC} ${CFLAGS} -o ${_SDIR}/taskq-check ${_SDIR}/taskq-check.c \
		xlib.o file_exists.o

clean distclean:
	rm -f *.o *.a *.so*
	rm -f ${_LDIR}/*.so
	rm -f ${_LDIR}/*.o
	rm -f ${_SDIR}/boottime
	rm -f ${_SDIR}/chkprog
	rm -f ${_SDIR}/cpu
	rm -f ${_SDIR}/getkey
	rm -f ${_SDIR}/lcdd
	rm -f ${_SDIR}/loadavg
	rm -f ${_SDIR}/stristr
	rm -f ${_SDIR}/uplinkd
	rm -f ${_SDIR}/trafficd
	rm -f ${_SDIR}/vmware-detect
	rm -f ${_SDIR}/sophos_scan
	rm -f ${_SDIR}/iosh
	rm -f ${_SDIR}/login
	rm -f ${_SDIR}/banner
	rm -f ${_SDIR}/taskq-check
	rm -f ${_SDIR}/syslog-filter
