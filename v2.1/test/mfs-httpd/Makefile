SSL_TREE =	/usr
SSL_DEFS =	-DUSE_SSL
SSL_INC =	-I${SSL_TREE}/include
SSL_LIBS =	-L${SSL_TREE}/lib -lssl -lcrypto


BINDIR =	/usr/local/sbin
MANDIR =	/usr/local/man
CC =		gcc
CDEFS =		${SSL_DEFS} ${SSL_INC}
CFLAGS =	-Os -w -march=i386 ${CDEFS}
LDFLAGS =	-s
LDLIBS =	${SSL_LIBS} ${SYSV_LIBS} ${CRYPT_LIB}

all:		clean httpd

httpd:	httpd.o match.o tdate_parse.o
	${CC} ${CFLAGS} ${LDFLAGS} httpd.o match.o tdate_parse.o ${LDLIBS} -o httpsd

httpd.o:	httpd.c match.h tdate_parse.h mime_encodings.h mime_types.h
	${CC} ${CFLAGS} -c httpd.c

match.o:	match.c match.h
	${CC} ${CFLAGS} -c match.c

tdate_parse.o:	tdate_parse.c tdate_parse.h
	${CC} ${CFLAGS} -c tdate_parse.c

mime_encodings.h: mime_encodings.txt
	rm -f mime_encodings.h
	sed < mime_encodings.txt > mime_encodings.h \
	  -e 's/#.*//' -e 's/[ 	]*$$//' -e '/^$$/d' \
	  -e 's/[ 	][ 	]*/", 0, "/' -e 's/^/{ "/' -e 's/$$/", 0 },/'

mime_types.h: mime_types.txt
	rm -f mime_types.h
	sed < mime_types.txt > mime_types.h \
	  -e 's/#.*//' -e 's/[ 	]*$$//' -e '/^$$/d' \
	  -e 's/[ 	][ 	]*/", 0, "/' -e 's/^/{ "/' -e 's/$$/", 0 },/'

cert:		httpd.pem
httpd.pem:	httpd.cnf
	openssl req -new -x509 -days 365 -nodes -config httpd.cnf -out httpd.pem -keyout httpd.pem
	openssl x509 -subject -dates -fingerprint -noout -in httpd.pem
	chmod 600 httpd.pem


install:	all
	rm -f ${BINDIR}/httpd ${BINDIR}/htpasswd
	-mkdir -p ${BINDIR}
	cp httpd htpasswd ${BINDIR}
	rm -f ${MANDIR}/man8/httpd.8 ${MANDIR}/man1/htpasswd.1
	-mkdir -p ${MANDIR}/man8
	cp httpd.8 ${MANDIR}/man8
	-mkdir -p ${MANDIR}/man1
	cp htpasswd.1 ${MANDIR}/man1

clean:
	rm -f httpsd mime_encodings.h mime_types.h htpasswd httpd.rnd *.o core core.* *.core
