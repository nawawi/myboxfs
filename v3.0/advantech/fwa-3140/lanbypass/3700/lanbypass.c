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

/* data structure definition */
#define LANBYPASS_MAX_GROUP 4
#define GPIOBASE  0x4080

typedef unsigned char uchar;

struct s_gpio_ctrl {
  unsigned int addr;
  unsigned int mask;
};


static const struct s_gpio_ctrl gpio_ctrl[LANBYPASS_MAX_GROUP] = {
  {GPIOBASE + 0x0C, 1 << 28},	/* GPIO 28 */
  {GPIOBASE + 0x38, 1 << 1 },	/* GPIO 33 */
  {GPIOBASE + 0x0C, 1 << 25},	/* GPIO 25 */
  {GPIOBASE + 0x0C, 1 << 27},	/* GPIO 27 */
};

/* local function declaration */
static void usage(void);
static void lb_group_show(int i);
static void lb_pw_show();
static void lb_gpio_set(uchar i);
static void lb_gpio_clear(uchar i);

/* global function implementation */
int main(int argc, char *argv[])
{
  uchar group;
  unsigned int data;
  unsigned int status;
  unsigned int b;
  char c;
  int i;

  if (iopl(3)) {
    perror(NULL);
    exit(1);
  }

  if (argc == 1) {
    usage();
    exit(2);
  }

  while ((c = getopt(argc, argv, "hse:d:p:")) != EOF) {
    switch (c) {
    case 'h':
      usage();
      break;
    case 's':
      /* read lan bypass statis */
      printf("Lan Bypass Status\n");
      printf("=======================\n");
      for (i = 0; i < 2; i++) {
	lb_group_show(i);
      }

      /* read power on/off lan bypass statis */
      lb_pw_show();
      break;
    case 'e':
      {
	group = atoi(optarg);
	lb_gpio_clear(group);
      }
      break;
    case 'd':
      {
	group = atoi(optarg);
	lb_gpio_set(group);
      }
      break;
    case 'p':
      {
        uchar s;

	s = atoi(optarg);
        if(s) {
          lb_gpio_set(2);
          lb_gpio_clear(3);
        } else {
          lb_gpio_clear(2);
    	  lb_gpio_set(3);
        }
      }
      break;
    default:
      usage();
      exit(3);
    }
    
  }

  exit(0);
}

/* local function implementation */
static void usage(void)
{
  printf("Usage: ./lanbypass { -e group_id | -d group_id | -p power_on -s }\n");
  printf("    where group_id from 0 to %d\n", LANBYPASS_MAX_GROUP - 1);
  printf("    -e enable lan bypass\n");
  printf("    -d disable lan bypass\n");
  printf("    -p power on/off\n");
  printf("    -s show all lan bypass status\n");
}

static void lb_group_show(int i)
{
  unsigned int data;
  unsigned int status;

  data = inl_p(gpio_ctrl[i].addr);
  status = data & gpio_ctrl[i].mask;
  if (status > 0) {
    printf("Group %d:\tDISABLE\n",i);
  } else {
    printf("Group %d:\tENABLE\n",i);
  }

}

static void lb_pw_show()
{
  unsigned int data;
  unsigned int status;

  data = inl_p(gpio_ctrl[2].addr);
  status = data & gpio_ctrl[2].mask;
  if (status > 0) {
    printf("Power On/Off Lan Bypass Control: On\n");
  } else {
    printf("Power On/Off Lan Bypass Control: Off\n");
  }

}

static void lb_gpio_set(uchar i)
{
  unsigned int b;

  b = inl_p(gpio_ctrl[i].addr);
  b |= gpio_ctrl[i].mask;
  outl_p(b, gpio_ctrl[i].addr);
}

static void lb_gpio_clear(uchar i)
{
  unsigned int b;

  b = inl_p(gpio_ctrl[i].addr);
  b &= ~gpio_ctrl[i].mask;
  outl_p(b, gpio_ctrl[i].addr);
}

