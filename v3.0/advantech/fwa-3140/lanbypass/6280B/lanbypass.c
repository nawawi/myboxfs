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
 * Description:	Setup lan bypass or not
 * Author:	Cplus Shen
 *
 * Usage: ./lanbypass { -e group_id | -d group_id | -s }
 * Usage: 	./lanbypass { -e group_id | -d group_id | -s group_id }
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

#define LANBYPASS_MAX_GROUP 6
typedef unsigned char uchar;

struct s_gpio_ctrl{
  unsigned short addr;
  unsigned char mask; 
  unsigned char high;
};

struct s_cmos_ctrl{
  unsigned short addr;
  unsigned char mask; 
};

static const struct s_gpio_ctrl gpio_ctrl[LANBYPASS_MAX_GROUP] = 
{
  { 0x4b8,0x20,0}, /* GPIO 37 */
  { 0x4b8,0x40,0}, /* GPIO 38 */
  { 0x4b8,0x80,0}, /* GPIO 39 */
  { 0x4b9,0x01,0}, /* GPIO 40 */
  { 0x4b9,0x08,0}, /* GPIO 43 */
  { 0x48e,0x80,0}, /* GPIO 23 */
};

static const struct s_cmos_ctrl cmos_ctrl[LANBYPASS_MAX_GROUP] = 
{
  { 0x5b,0x01}, 
  { 0x5b,0x02}, 
  { 0x5b,0x04}, 
  { 0x5b,0x08}, 
  { 0x5c,0x10}, 
  { 0x5c,0x20}, 
};

static void usage(void);

int main(int argc, char *argv[])
{
    uchar b;
    uchar group;
    uchar data;
    uchar status;
 

    if (iopl(3)) {
	perror(NULL);
	exit(1);
    }

    if (argc < 2) {
	usage();
 	exit(2);
    }

    switch (argv[1][1]) {
        case 's': 
            {
		int i;
		
		if (argc != 2) {
		    usage();
		    exit(3);
                }
		printf("Lan Bypass Status\n");
		printf("=======================\n");
		for (i=0; i < LANBYPASS_MAX_GROUP; i++) {
		    data = inb_p(gpio_ctrl[i].addr);
		    status = data & gpio_ctrl[i].mask;
		    if ( gpio_ctrl[i].high ) {
			if (status > 0) {
		            printf("Group %d:\tENABLE\n",i );
                        } else {
		            printf("Group %d:\tDISABLE\n",i );
                        } 
		    } else {
			if (status > 0) {
		            printf("Group %d:\tDISABLE\n",i );
                        } else {
		            printf("Group %d:\tENABLE\n",i );
                        } 

                    }
                }
            }
	    break;
        case 'd': 
            {
                uchar b;
                uchar c;

                group = atoi(argv[2]);
                b = inb_p(gpio_ctrl[group].addr); 


		if (gpio_ctrl[group].high) {
		    b &= ~gpio_ctrl[group].mask;
                } else {
		    b |= gpio_ctrl[group].mask;
                }
		outb_p(b,gpio_ctrl[group].addr);	

		/* CMOS, Support by Ted */
		outb_p(cmos_ctrl[group].addr,0x70);
		c = inb_p(0x71);
	        c &= ~cmos_ctrl[group].mask;
	        outb_p(c,0x71);
            }
	    break;
        case 'e':
            {
                uchar b;
		uchar c;

                group = atoi(argv[2]);
                b = inb_p(gpio_ctrl[group].addr); 

		if (gpio_ctrl[group].high) {
		    b |= gpio_ctrl[group].mask;
                } else {
		    b &= ~gpio_ctrl[group].mask;
                }
		outb_p(b,gpio_ctrl[group].addr);	

		/* CMOS, Support by Ted */
		outb_p(cmos_ctrl[group].addr,0x70);
		c = inb_p(0x71);
	        c |= cmos_ctrl[group].mask;
	        outb_p(c,0x71);
            }
	    break;
    }
    exit(0);
}

static void usage(void)
{
    printf("Usage: ./lanbypass { -e group_id | -d group_id | -s }\n");
    printf("    where group_id from 0 to %d\n",LANBYPASS_MAX_GROUP-1);
    printf("    -e enable lan bypass\n");
    printf("    -d disable lan bypass\n");
    printf("    -s show all lan bypass status\n");
}
