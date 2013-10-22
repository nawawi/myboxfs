/*
  Lan Bypass Control Program
  (C) 2005  Cplus Shen <cplus.shen@advantech.com.tw>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* 
 * File:	lanbypass.c   
 * Description:	Setup lan by pass or not
 * Author:	Cplus Shen
 *
 * Usage:	./lanbypass [on | off]
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>
#include <errno.h>

#define GPIO_LANBYPASS_ADDR 0x48F
#define GPIO_LANBYPASS_BITS 1
#define uchar unsigned char

static usage(uchar b);

int main(int argc, char *argv[])
{
    uchar b;

    if (iopl(3)) {
	perror(NULL);
	exit(1);
    }

    b = inb_p(GPIO_LANBYPASS_ADDR);

    if (argc != 2) {
	usage(b);
    }

    if (strcmp("off", argv[1]) == 0) {
	b |= 1 << GPIO_LANBYPASS_BITS;
	outb_p(b, GPIO_LANBYPASS_ADDR);
    } else if (strcmp("on", argv[1]) == 0) {
	b &= ~(1 << GPIO_LANBYPASS_BITS);
	outb_p(b, GPIO_LANBYPASS_ADDR);
    } else {
	usage(b);
    }
    exit(0);
}

static usage(uchar b)
{
    printf("Usage: ./lanbypass [on | off]\n");
    if (b & (1 << GPIO_LANBYPASS_BITS)) {
	printf("Lan No Bypass\n");
    } else {
	printf("Lan Bypass\n");
    }
    exit(1);
}
