ifndef $(DESTDIR)
        DESTDIR="/usr/local/bin"
endif
CFLAGS=-I/usr/local/include -lcrypto -lcurl -ldl -lidn -lssl -L/usr/lib -lz 

all:    squidclam.c config.c clamdscan.c
	gcc -Wall  -I/usr/local/include -lcrypto -lcurl -ldl -lidn -lssl -L/usr/lib -lz  -s -o squidclam squidclam.c config.c clamdscan.c

gdb:    squidclam.c config.c clamdscan.c
	gcc -Wall  -I/usr/local/include -lcrypto -lcurl -ldl -lidn -lssl -L/usr/lib -lz  -g -o squidclam squidclam.c config.c clamdscan.c

debug:    squidclam.c config.c clamdscan.c
	gcc -Wall  -DDEBUG -I/usr/local/include -lcrypto -lcurl -ldl -lidn -lssl -L/usr/lib -lz  -g -O2 -o squidclam squidclam.c config.c clamdscan.c

clean: 
	rm -f squidclam src/Makefile

install: squidclam
	cp squidclam $(DESTDIR)/.

tar: clean 
	tar --owner=nobody --group=nobody -cvzC ../.. -f ../../release/squidclam-0.22.tar.gz squidclam-0.22 
	md5sum ../../release/squidclam-0.22.tar.gz | tee ../../release/squidclam-0.22.md5
