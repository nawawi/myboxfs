NEWTLIBS=/usr/lib/libnewt.a  /usr/lib/libslang.a -lproc

all: clean
	@gcc -O2 -w -mcpu=i686 -march=i686 -fno-strict-aliasing -o login login.c strings.c md5.c -lsqlite -lreadline -lncurses -lcrypt $(NEWTLIBS)
	@gcc -O2 -w -mcpu=i686 -march=i686 -fno-strict-aliasing -o login_shell shell.c
	@strip login login_shell

clean:
	rm -f login login_shell
