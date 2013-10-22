/*
* Copyright (c) 2004 Alexander Eremin <xyzyx@rambler.ru>
*
* Mini ipcalc implementation for busybox
*
* By Jordan Crouse <jordan@cosmicpenguin.net>
*    Stephan Linz  <linz@li-pro.net>
*
* This is a complete reimplentation of the ipcalc program
* from Redhat.  I didn't look at their source code, but there
* is no denying that this is a loving reimplementation
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef unsigned long uint32;

int verbose, output;
static void check(char *err);
static int analyze(char *addr);
  
void ipcheck(char *arg)
{
output = 0;
analyze(arg);
}

static void check(char *err)
{
  fflush(stdout);
//  if (err) fprintf(stderr, "%% %s\r\n", err);
  output=1;
}

//static int rfc1918(uint32 ipaddr, int verbose);
//static char *fmtip(uint32 val);
//static char *fmtip2(uint32 val);
static int makemask(int oc[4], uint32 *netmask);
static int powoftwo(uint32 val);

#define ADDRTYPE (isnw ? "network" : (isbc ? "broadcast" : "host"))

static int analyze(char *addr)
{
  uint32 ipaddr=0, netmask, network, bcast;
  int oc[4], slash, cla, isnw, isbc, i;
  char *p;

  i = sscanf(addr, "%d.%d.%d.%d", &oc[0], &oc[1], &oc[2], &oc[3]);
  if (i != 4) check("%% IP address format error");
  for (i = 0; i < 4; i++)
      if (oc[i] < 0 || oc[i] > 255)
          check("%% Octet out of range");
      else ipaddr |= oc[i] << 8*(3-i);
  
  if ((p = strchr(addr, '/'))) { /* slash specified? */
      if (strchr(p, '.')) { /* netmask specified? */
          i = sscanf(p, "/%d.%d.%d.%d", &oc[0], &oc[1], &oc[2], &oc[3]);
          if (i != 4) check("%% Net mask format error");
          i = makemask(oc, &netmask);
          if (i == 0) check("%% Invalid net mask");
          else slash = 32 - i;
      }
      else { /* slash specified */
          i = sscanf(p, "/%d", &slash);
          if (i != 1 || slash < 0 || slash > 32)
              check("%% /n out of range, 0<=n<=32");
          if (slash == 32)	{
		netmask = 0xffffffff;
	  }
          else {
		netmask = 1 << (31-slash);
		netmask = ~((netmask | netmask) - 1);
          }
      }
      switch(slash) {
          case  8: cla='A'; break;
          case 16: cla='B'; break;
          case 24: cla='C'; break;
          default: cla=0; break;
      }
  }
  else { /* no netmask specified */
      int first = (ipaddr >> 24) & 0xFF; /* first octet */
      if ((first&0x80) == 0x00) cla='A', slash=8, netmask=0xFF000000; /*A*/
      else if ((first&0xC0) == 0x80) cla='B', slash=16, netmask=0xFFFF0000;
      else if ((first&0xE0) == 0xC0) cla='C', slash=24, netmask=0xFFFFFF00;
      else if ((first&0xF0) == 0xE0) cla='D', slash=-1, netmask=0; /* D */
      else cla='E', slash=-1, netmask=0; /* class E: reserved */
  }
  
  /* all the rest is easy... */
  
  network = ipaddr & netmask;                /* network address */
  bcast = ipaddr | (0xFFFFFFFF & ~netmask);  /* broadcast address */
  isnw = (ipaddr == network);                /* host or network? */
  isbc = (ipaddr == bcast);                  /* broadcast address? */

  /* finally, print what we've figured out */

    if(output!=0) 
    {
	return 1;
    }
    else
    return 0;
}
/*
int rfc1918(uint32 ipaddr, int verbose)  
// see RFC 1918 
{
  int ispriv; ipaddr >>= 16;
  ispriv = ((ipaddr & 0xff00) == (uint32) 10*256) ||
           ((ipaddr & 0xfff0) == (uint32) 172*256+16) ||
           ((ipaddr & 0xffff) == (uint32) 192*256+168);
  //if (ispriv && verbose)
      printf("This is a private internet address (see RFC 1918).\n");
 // return ispriv;
}
*/
/* create net mask from octets, return slash value */
static int makemask(int oc[4], uint32 *netmask)
{
  int i;
  uint32 mask = 0;
  
  for (i = 0; i < 4; i++)
      if (oc[i] < 0 || oc[i] > 255) return 0;
      else mask |= oc[i] << 8*(3-i);
  if (netmask) *netmask = mask;
  
  /* Draw bit patterns to understand this code! */
  return powoftwo((~mask)^(2*(~mask)+1));
}
/*
static char *fmtip(uint32 val)
{
  static char buf[16];  
  // xxx.xxx.xxx.xxx\0 
  int i = sprintf(buf, "%d.%d.%d.%d",
           (val>>24)&0xff, (val>>16)&0xff, (val>>8)&0xff, val&0xff);
  return buf;
}

static char *fmtip2(uint32 val)
{
  int i, j;
  static char buf[80];
  // xxx.xxx.xxx.xxx   ........ ........ ........ ........\0 
  i = sprintf(buf, "%3d.%3d.%3d.%3d   ",  
  	(val>>24)&0xff, (val>>16)&0xff, (val>>8)&0xff, val&0xff);
  for (j=31; j >= 0; j--) {  
	buf[i++] = (val&(1<<j)) ? '1' : '0';
	if (j == 24 || j == 16 || j == 8) buf[i++] = ' ';
  }
  buf[i] = '\0';

  return buf;
}
*/
/* what power of two? assumes 2's complement */
static int powoftwo(uint32 val)
{
  int i;
  
  if (val & (val-1)) return 0; /* not a power of two */
  
  for (i = 0; (val&1) == 0; i++) val >>= 1;
  return i; /* val == 2^i */
}
