NEWTLIBS=/usr/lib/libsqlite.a

all: clean
	@gcc -O2 -w -march=i686 -fno-strict-aliasing -o login login.c md5.c $(NEWTLIBS)
	@gcc -O2 -w -march=i686 -fno-strict-aliasing -o chkprog chkprog.c popen.c
	@gcc -O2 -w -march=i686 -fno-strict-aliasing -o getkey getkey.c
	@gcc -O2 -w -march=i686 -fno-strict-aliasing -o logtail logtail.c
	@strip login chkprog getkey logtail

clean:
	rm -f login chkprog getkey initlog logtail
