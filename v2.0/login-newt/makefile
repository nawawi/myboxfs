NEWTLIBS=/usr/lib/libnewt.a  /usr/lib/libslang.a /usr/lib/libsqlite.a -lproc

all: clean
	@icc -O2 -w -march=i686 -fno-strict-aliasing -o login login.c strings.c md5.c -lreadline -lncurses -lcrypt $(NEWTLIBS)
	@icc -O2 -w -march=i686 -fno-strict-aliasing -o chkprog chkprog.c strings.c
	@strip login chkprog

clean:
	rm -f login chkprog
