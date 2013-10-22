/*
 * Copyright (c) 2004 Alexander Eremin <xyzyx@rambler.ru>
 *    
 * route
 *
 * Similar to the standard Unix route, but with only the necessary
 * parts for AF_INET
 *
 * Bjorn Wesen, Axis Communications AB
 *
 * Author of the original route: 
 *              Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              (derived from FvK's 'route.c     1.70    01/04/94')
 *
 * This program is free software; you can redistribute it
 * and/or  modify it under  the terms of  the GNU General
 * Public  License as  published  by  the  Free  Software
 * Foundation;  either  version 2 of the License, or  (at
 * your option) any later version.
 *
 * $Id: route.c,v 1.2 2000/12/22 19:30:07 andersen Exp $
 *
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/route.h>
#include <linux/param.h>  // HZ
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <ctype.h>
#include "../lib/ext.h"

#define _(x) x

#define RTACTION_ADD   1
#define RTACTION_DEL   2
#define RTACTION_HELP  3
#define RTACTION_FLUSH 4
#define RTACTION_SHOW  5

#define E_NOTFOUND      8
#define E_SOCK          7
#define E_LOOKUP        6
#define E_VERSION       5
#define E_USAGE         4
#define E_OPTERR        3
#define E_INTERN        2
#define E_NOSUPP        1

/* resolve XXX.YYY.ZZZ.QQQ -> binary */

static int
INET_resolve(char *name, struct sockaddr *sa)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)sa;
	
	sin->sin_family = AF_INET;
	sin->sin_port = 0;

	/* Default is special, meaning 0.0.0.0. */
	if (!strcmp(name, "default")) {
		sin->sin_addr.s_addr = INADDR_ANY;
		return (1);
	}
	/* Look to see if it's a dotted quad. */
	if (inet_aton(name, &sin->sin_addr)) {
		return 0;
	}
	/* guess not.. */
	return -1;
}

#if defined (SIOCADDRTOLD) || defined (RTF_IRTT)        /* route */
#define HAVE_NEW_ADDRT 1
#endif
#ifdef RTF_IRTT                 /* route */
#define HAVE_RTF_IRTT 1
#endif
#ifdef RTF_REJECT               /* route */
#define HAVE_RTF_REJECT 1
#endif

#if HAVE_NEW_ADDRT
#define mask_in_addr(x) (((struct sockaddr_in *)&((x).rt_genmask))->sin_addr.s_addr)
#define full_mask(x) (x)
#else
#define mask_in_addr(x) ((x).rt_genmask)
#define full_mask(x) (((struct sockaddr_in *)&(x))->sin_addr.s_addr)
#endif

/* add or delete a route depending on action */

static int
INET_setroute(int action, int options, char **args)
{
	struct rtentry rt;
	char target[128], gateway[128] = "NONE", netmask[128] = "default";
	int xflag, isnet;
	int skfd;

	xflag = 0;

	if (!strcmp(*args, "-net")) {
		xflag = 1;
		args++;
	} else if (!strcmp(*args, "-host")) {
		xflag = 2;
		args++;
	}
	if (*args == NULL)
		printf("%% Incomplete command\r\n");

	safe_strncpy(target, *args++, (sizeof target));

	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));


	if ((isnet = INET_resolve(target, &rt.rt_dst)) < 0) {
		fprintf(stderr, "%% cant resolve %s\n", target);
		return (1);
	}

	switch (xflag) {
		case 1:
			isnet = 1;
			break;
			
		case 2:
			isnet = 0;
			break;
			
		default:
			break;
	}
	
	/* Fill in the other fields. */
	rt.rt_flags = (RTF_UP | RTF_HOST);
	if (isnet)
		rt.rt_flags &= ~RTF_HOST;

	while (*args) {
		if (!strcmp(*args, "metric")) {
			int metric;
			
			args++;
			if (!*args || !isdigit(**args))
				printf("%% Incomplete command\r\n");
			metric = atoi(*args);
#if HAVE_NEW_ADDRT
			rt.rt_metric = metric + 1;
#else
			ENOSUPP("inet_setroute", "NEW_ADDRT (metric)");
#endif
			args++;
			continue;
		}

		if (!strcmp(*args, "netmask")) {
			struct sockaddr mask;
			
			args++;
			if (!*args || mask_in_addr(rt))
				printf("%% Incomplete command\r\n");
			safe_strncpy(netmask, *args, (sizeof netmask));
			if ((isnet = INET_resolve(netmask, &mask)) < 0) {
				fprintf(stderr, "%% cant resolve netmask %s\n", netmask);
				return (E_LOOKUP);
			}
			rt.rt_genmask = full_mask(mask);
			args++;
			continue;
		}

		if (!strcmp(*args, "gw") || !strcmp(*args, "gateway")) {
			args++;
			if (!*args)
				printf("%% Incomplete command\r\n");
			if (rt.rt_flags & RTF_GATEWAY)
				printf("%% Incomplete command\r\n");
			safe_strncpy(gateway, *args, (sizeof gateway));
			if ((isnet = INET_resolve(gateway, &rt.rt_gateway)) < 0) {
				fprintf(stderr, "%% cant resolve gw %s\n", gateway);
				return (E_LOOKUP);
			}
			if (isnet) {
				fprintf(stderr,
					_("%%  %s: cannot use a NETWORK as gateway!\n"),
					gateway);
				return (E_OPTERR);
			}
			rt.rt_flags |= RTF_GATEWAY;
			args++;
			continue;
		}

		if (!strcmp(*args, "mss")) {
			args++;
			rt.rt_flags |= RTF_MSS;
			if (!*args)
				printf("%% Incomplete command\r\n");
			rt.rt_mss = atoi(*args);
			args++;
			if (rt.rt_mss < 64 || rt.rt_mss > 32768) {
				fprintf(stderr, _("%% Invalid MSS.\n"));
				return (E_OPTERR);
			}
			continue;
		}

		if (!strcmp(*args, "window")) {
			args++;
			if (!*args)
				printf("%% Incomplete command\r\n");
			rt.rt_flags |= RTF_WINDOW;
			rt.rt_window = atoi(*args);
			args++;
			if (rt.rt_window < 128) {
				fprintf(stderr, _("%% Invalid window.\n"));
				return (E_OPTERR);
			}
			continue;
		}

		if (!strcmp(*args, "irtt")) {
			args++;
			if (!*args)
				printf("%% Incomplete command\r\n");
			args++;
#if HAVE_RTF_IRTT
			rt.rt_flags |= RTF_IRTT;
			rt.rt_irtt = atoi(*(args - 1));
			rt.rt_irtt *= (HZ / 100);	/* FIXME */
#if 0				/* FIXME: do we need to check anything of this? */
			if (rt.rt_irtt < 1 || rt.rt_irtt > (120 * HZ)) {
				fprintf(stderr, _("%% Invalid initial rtt.\n"));
				return (E_OPTERR);
			}
#endif
#else
			ENOSUPP("inet_setroute", "RTF_IRTT");
#endif
			continue;
		}

		if (!strcmp(*args, "reject")) {
			args++;
#if HAVE_RTF_REJECT
			rt.rt_flags |= RTF_REJECT;
#else
			ENOSUPP("inet_setroute", "RTF_REJECT");
#endif
			continue;
		}
		if (!strcmp(*args, "mod")) {
			args++;
			rt.rt_flags |= RTF_MODIFIED;
			continue;
		}
		if (!strcmp(*args, "dyn")) {
			args++;
			rt.rt_flags |= RTF_DYNAMIC;
			continue;
		}
		if (!strcmp(*args, "reinstate")) {
			args++;
			rt.rt_flags |= RTF_REINSTATE;
			continue;
		}
		if (!strcmp(*args, "device") || !strcmp(*args, "dev")) {
			args++;
			if (rt.rt_dev || *args == NULL)
				printf("%% Incomplete command\r\n");
			rt.rt_dev = *args++;
			continue;
		}
		/* nothing matches */
		if (!rt.rt_dev) {
			rt.rt_dev = *args++;
			if (*args)
				printf("%% Incomplete command\r\n");	/* must be last to catch typos */
		} else
			printf("%% Incomplete command\r\n");
	}

#if HAVE_RTF_REJECT
	if ((rt.rt_flags & RTF_REJECT) && !rt.rt_dev)
		rt.rt_dev = "lo";
#endif

	/* sanity checks.. */
	if (mask_in_addr(rt)) {
		unsigned long mask = mask_in_addr(rt);
		mask = ~ntohl(mask);
		if ((rt.rt_flags & RTF_HOST) && mask != 0xffffffff) {
			fprintf(stderr,
				_("%% netmask %.8x doesn't make sense with host route\n"),
				(unsigned int)mask);
			return (E_OPTERR);
		}
		if (mask & (mask + 1)) {
			fprintf(stderr, _("%% bogus netmask %s\n"), netmask);
			return (E_OPTERR);
		}
		mask = ((struct sockaddr_in *) &rt.rt_dst)->sin_addr.s_addr;
		if (mask & ~mask_in_addr(rt)) {
			fprintf(stderr, _("%% netmask doesn't match route address\n"));
			return (E_OPTERR);
		}
	}
	/* Fill out netmask if still unset */
	if ((action == RTACTION_ADD) && rt.rt_flags & RTF_HOST)
		mask_in_addr(rt) = 0xffffffff;

	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return (E_SOCK);
	}
	/* Tell the kernel to accept this route. */
	if (action == RTACTION_DEL) {
		if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
			perror("SIOCDELRT");
			close(skfd);
			return (E_SOCK);
		}
	} else {
		if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
			perror("SIOCADDRT");
			close(skfd);
			return (E_SOCK);
		}
	}
	
	/* Close the socket. */
	(void) close(skfd);
	return (0);
}

int route_main(int argc, char **argv)
{
	int what = 0;

	argc--;
	argv++;

	if (*argv == NULL) {
		//displayroutes();
		fprintf(stderr, "%% Incomplete command\r\n");
		printf("%% Incomplete command\r\n");
	} else {
		/* check verb */
		if (!strcmp(*argv, "add"))
			what = RTACTION_ADD;
		else if (!strcmp(*argv, "del") || !strcmp(*argv, "delete"))
			what = RTACTION_DEL;
		else if (!strcmp(*argv, "flush"))
			what = RTACTION_FLUSH;
		else
			printf("%% Incomplete command\r\n");
	}

	INET_setroute(what, 0, ++argv);

	return(0);
}


void showroute(void)
{
	char buff[256];
	int  nl = 0 ;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	int flgs, ref, use, metric;
	int via;
	int viagw;
	//char flags[4];
	unsigned long int d,g,m;

	char sdest[16], sgw[36];
	FILE *fp = xfopen("/proc/net/route", "r");
	while( fgets(buff, sizeof(buff), fp) != NULL ) {
		if(nl) {
			int ifl = 0;
			while(buff[ifl]!=' ' && buff[ifl]!='\t' && buff[ifl]!='\0')
				ifl++;
			buff[ifl]=0;    /* interface */
			if(sscanf(buff+ifl+1, "%lx%lx%d%d%d%d%lx",
			   &d, &g, &flgs, &ref, &use, &metric, &m)!=7) {
				printf( "Unsuported kernel route format\n");
			}
			if(nl==1) {
    via=0;
    viagw=0;

    printf("Codes: C -connected, S -static, I -IGRP, R -RIP, B -BGP\n");			
    printf("\tD -EIGRP, O -OSPF, * -candidate default\n");
//    printf("Gateway of last resort is not set\n");
			}

		//	ifl = 0;        /* parse flags */
		/*	if(flgs&1)
				flags[ifl++]='U';
			if(flgs&2)
				flags[ifl++]='G';
			if(flgs&4)
				flags[ifl++]='H';
			flags[ifl]=0;*/
			dest.s_addr = d;
			gw.s_addr   = g;
			mask.s_addr = m;
if (dest.s_addr==0)
{
    strcpy(sdest,"*"); via=0;
}
else
{
 strcpy(sdest,inet_ntoa(dest)); via=1; 
}
if (gw.s_addr==0)
{
strcpy(sgw,"is directly connected");viagw=0;
}
else 
{
strcpy(sgw,inet_ntoa(gw)); viagw=1;
}
//strcpy(sdest,  (dest.s_addr==0 ? "default" : inet_ntoa(dest)));
//strcpy(sgw,    (gw.s_addr==0   ? "is directly connected" : inet_ntoa(gw)));
			
		//	printf("C %-16s%s, %s\n", sdest, sgw,buff);
		if(!(via && viagw))	
		//printf("C %-16s%s, %s\n", sdest, sgw,buff);
		printf("> %s %s, %s\n", sdest, sgw,buff);
		else printf("> %s via %s, %s, [m:%d]\n", sdest, sgw,buff,metric);
		
		/*	printf("C %-16s%-16s%-16s%-6s%-6d %-2d %7d %s\n",
				sdest, sgw,
				inet_ntoa(mask),
				flags, metric, ref, use, buff);
	*/	}
	nl++;
	}
}
