#include <unistd.h>
#include <stdlib.h>

int main(void) {
	unsetenv("source");
	unsetenv("OLDPWD");
	unsetenv("initrd");
	unsetenv("vga");
	unsetenv("hd-boot");
	unsetenv("hd-strg");
	unsetenv("hd-swap");
	unsetenv("post-single");
	unsetenv("LOGNAME");
	unsetenv("MAIL");
	setenv("MAILCHECK","0",1);
	setenv("HOME","/",1);
	setenv("USER","mfs",1);
	setenv("SHELL","/bin/bash",1);
	setenv("PATH","/bin:/service/tools:/srvice/init",1);
	setenv("INPUTRC","/etc/inputrc",1);
	setenv("BASH_ENV","/.bashrc",1);
	setenv("HISTFILE","/.bash_history",1);
	setenv("HISTFILESIZE","100",1);
	setenv("HISTSIZE","100",1);
	setenv("PS1","(\\h)# ",1);
	chdir("/");
	execl("/bin/bash","/bin/bash",0);
	return(0);
}
