/*
 * $Copyright: (c) 2004 Advantech Corp.
 * All Rights Reserved.$
 *
 * File:	lcm100.c  
 * Purpose:	LCM100 Demo code
 *
 * Some of the code are modified from LCDProc0.4.5
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcm100.h"

static int fd;

int 
LCM100_init()
{
	struct termios portset;
	char device[256] = "/dev/lcd";
	int speed=B2400;
	fd = open ( device, O_RDWR|O_NOCTTY|O_NDELAY);
	if(fd == -1){
		fprintf(stderr,"LCM100_init: failed (%s)\n",strerror(errno));
		return -1;
	}
	tcgetattr(fd, &portset);
	cfmakeraw(&portset);
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, B0);
	tcsetattr(fd, TCSANOW, &portset);
	return fd;

}

void 
LCM100_close()
{
    close (fd);
}   

void 
LCM100_clear()
{
	char out[4];
	snprintf(out, sizeof(out), "%c%c",LCD_CMD,LCD_CLEAR);
	write(fd,out,2);
	return;
}

void 
LCM100_hide_cursor()
{
	char out[4];
	snprintf(out, sizeof(out), "%c%c",LCD_CMD,LCD_HIDE_CURSOR);
	write(fd,out,2);
	return;

}

void 
LCM100_show_cursor(int type)
{
	char out[4];
	if(type==1){
		snprintf(out, sizeof(out), "%c%c",LCD_CMD,LCD_ENABLE_BLOCK_CURSOR);
	}else{
		snprintf(out, sizeof(out), "%c%c",LCD_CMD,LCD_ENABLE_UNDER_CURSOR);
	}
	write(fd,out,2);
	return;
}

void
LCM100_string (int x, int y, char string[])
{
	int i;
	char c;
	char out[16];
	
	snprintf (out, sizeof(out), "%c%c", LCD_CMD,128+(64*(y))+x);
	write (fd, out, 2);
	for (i = 0; string[i]; i++) {
		c = string[i];
		switch(c)
		{
			case '\254': 
				c='#';
				break;
		}
		// write out string
		snprintf(out, sizeof(out), "%c",c);
		write(fd,out,1);
	}
		
}

void
LCM100_char (int x, int y, char c)
{
		  
	char chr[2];
	chr[0] = c;
	chr[1] = 0;
    LCM100_string (x, y, chr);
}

void 
LCM100_set_char (int n, char *dat)
{
	char out[4];
    int row, col;
	int letter;

	if (n < 0 || n > 7)
		return;
	n = 64 + (8 * n);
	if (!dat)
		return;

	snprintf (out, sizeof (out), "%c%c", LCD_CMD, n);
	write (fd, out, 2);

	for (row = 0; row < LCM100_CELLHGT; row++) {
		letter = 1;
		for (col = 0; col < LCM100_CELLWID; col++) { 
			letter <<= 1; 
			letter |= (dat[(row * LCM100_CELLWID) + col] > 0); 
		} 
		snprintf (out, sizeof (out), "%c", letter); 
		write (fd, out, 1); 
	}
}
void
LCM100_init_vbar ()
{
	char a[] = {
		0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
	    1, 1, 1, 1, 1,
	};
    char b[] = {
	    0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char c[] = { 
		0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
    char d[] = { 
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char f[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char g[] = {
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	LCM100_set_char (1, a);
	LCM100_set_char (2, b);
	LCM100_set_char (3, c);
	LCM100_set_char (4, d);
	LCM100_set_char (5, e);
	LCM100_set_char (6, f);
	LCM100_set_char (7, g);
}


int main(int argc,char** argv)
{
	char out[4];
	unsigned char b;
	int i=0, x=0, j=0;
	int wait;

	fd = LCM100_init();
	if(fd < 0 ){ 
		exit(-1);
	}
	printf("Press any key on LCM100 keypad to advance to next test!\n");
	printf("Clear LCD\n");
	LCM100_clear();
	printf("Show LOGO\n");
	LCM100_string(0,0,"    Advantech\0");
	LCM100_string(0,1,"     LCM100\0");
	wait=1;
    while (wait){
		read(fd,&b,1);
		if(b==(unsigned char)253){wait=0;}
	}

	printf("Show Test\n");
    LCM100_string(0,0,"1234567890123456\0");
    LCM100_string(0,1,"1234567890123456\0");
	
	wait=1;
    while (wait){
		read(fd,&b,1);
		if(b==(unsigned char)253){wait=0;}
	}
	LCM100_clear();
	LCM100_init_vbar();

		LCM100_string(0,0,"\001\0");
		LCM100_string(1,0,"\002\0");
		LCM100_string(2,0,"\003\0");
		LCM100_string(3,0,"\004\0");
		LCM100_string(4,0,"\005\0");
		LCM100_string(5,0,"\006\0");
		LCM100_string(6,0,"\007\0");

	wait=1;
    while (wait){
		read(fd,&b,1);
		if(b==(unsigned char)253){wait=0;}
	}
	printf("Entering keypad test mode\n");
	LCM100_clear();
	while(1){
		read(fd,&b,1);
		i++;
		if( b == (unsigned char)253){
			read(fd,&b,1);
			printf("%d:b=0x%02x:",i,b);
			if(b & 0x1) { printf("Key UP ");   LCM100_string(0,x=x++%2,"UP   ");}
			if(b & 0x2) { printf("Key DOWN "); LCM100_string(0,x=x++%2,"DOWN ");}
			if(b & 0x4) { printf("Key LEFT "); LCM100_string(0,x=x++%2,"LEFT ");}
			if(b & 0x8) { printf("Key RIGHT ");LCM100_string(0,x=x++%2,"RIGHT");}
			if(b & 0x10){ printf("Key ENTER ");LCM100_string(0,x=x++%2,"ENTER");}
			printf("\n");
		}
	}
	LCD100_close();
}
// vim: ts=4
