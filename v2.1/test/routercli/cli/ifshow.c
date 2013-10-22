/*
 * Copyright (c) 2004 Alexander Eremin <xyzyx@rambler.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

    
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include "../lib/ext.h"

#define inaddrr( x ) ( *( struct in_addr * )&ifr->x[ sizeof( sa.sin_port ) ] )
#define IFRSIZE      ( ( int )( size * sizeof( struct ifreq ) ) )

int ethshow(char *ifname)
{
    struct ifreq ifr;
    int i;
    int intf;
    int fd;
    int up = 0;
    int running = 0;
    int noarp;
    int multi;
    int bcast;
    intf=0;
    fd = socket(AF_INET,SOCK_DGRAM, 0);
       if (fd >= 0) 
       {
    	    strcpy(ifr.ifr_name, ifname);
	    if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0) 
	    {
		/* flags */
		if (ifr.ifr_flags & IFF_UP) up = 1; else up = 0;
		if (ifr.ifr_flags & IFF_RUNNING) running = 1; else running = 0;
		if (ifr.ifr_flags & IFF_NOARP) noarp = 1; else noarp = 0;
		if (ifr.ifr_flags & IFF_MULTICAST) multi = 1; else multi = 0;
		if (ifr.ifr_flags & IFF_BROADCAST) bcast = 1; else bcast = 0;
		
	    }
	    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) 
	    {
		printf( "Ethernet %s is ", ifname);
		printf("%s, line protocol is %s\n",up ? "up": "down", running ? "up" : "down");
		intf=1;	
		printf("\tInternet address is ");
		for( i=2; i<6; i++)
    		{
            	    unsigned char val = (unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[i];
        	    printf( "%d%s", val, i==5?" ":".");
    		}
	    }
/*	    if (ioctl(fd,SIOCGIFBRDADDR, &ifr) >= 0) 
	    {
		printf( "\tBcast ");
		for( i=2; i<6; i++)
		{
        	    unsigned char val = (unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[i];
    		    printf( "%d%s", val, i==5?" ":".");
    		}
	    }
*/
	    if (ioctl(fd,SIOCGIFNETMASK, &ifr) >= 0) 
	    {
		printf( ", subnet mask is ");
		for( i=2; i<6; i++)
		{
        	    unsigned char val = (unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[i];
    		    printf( "%d%s", val, i==5?" ":".");
    		}
		printf( "\n");
	    }
/*	    if (intf)
	    {
		printf( noarp ? "ARP, " : "No ARP, ");
		printf( bcast ? "Broadcast, " : "no Broadcast, ");
		printf( multi ? "Multicast" : "no Multicast");
		printf("\n");
	    }
*/
	    if (ioctl(fd,SIOCGIFHWADDR, &ifr) >= 0) 
	    {
		printf( "\tHardware address is ");
		printf("%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
		(unsigned char)ifr.ifr_hwaddr.sa_data[0],
		(unsigned char)ifr.ifr_hwaddr.sa_data[1],
		(unsigned char)ifr.ifr_hwaddr.sa_data[2],
		(unsigned char)ifr.ifr_hwaddr.sa_data[3],
		(unsigned char)ifr.ifr_hwaddr.sa_data[4],
		(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
	    }
	    if (ioctl(fd, SIOCGIFMAP, &ifr) >= 0)
            {
                printf( ", irq %d, ", ifr.ifr_map.irq);
                printf( "base addr 0x%x\n",ifr.ifr_map.base_addr);
        //      printf( "port %d, ", ifr.ifr_map.port);
        //      printf( "dma %x, ", ifr.ifr_map.dma);
        //	printf( "memory %lx-%lx\n", ifr.ifr_map.mem_start,ifr.ifr_map.mem_end);
        	
	    }
	    if (ioctl(fd,SIOCGIFMTU, &ifr) >= 0) 
	    {
		printf( "\tMTU %d bytes, ",ifr.ifr_mtu );
	    }
	    if (ioctl(fd,SIOCGIFMETRIC, &ifr) >= 0) 
	    {
		printf( "metric %u, ",ifr.ifr_metric ? ifr.ifr_metric : 1 );
	    }
	
	    if (ioctl(fd,SIOCGIFTXQLEN, &ifr) >= 0) 
	    {
		printf( "txqueuelen %d\n",ifr.ifr_qlen );
	    }
/*	    if (ioctl(fd,SIOCGIFFLAGS, &ifr) >= 0) 
	    {
		printf( "flags %d, ",ifr.ifr_flags );
	    }
*/	    
	else printf("%% Invalid interface\n");    
       }
       info(ifname);
      close(fd );
    return 0;
}
int info(char *iface)
{
    int i;
    int f = 1;
    FILE *fp;
    char *colon = NULL, device[64],rbytes[64],rpackets[64],rerrors[64],rdrop[64],rfifo[64],rframe[64],rcompressed[64],rmulticast[64],tbytes[64],tpackets[64],terrors[64],tdrop[64],tfifo[64],tcolls[64],tcarrier[64],tcompressed[64];
    char ibuf[1024];
    char ibuf2[20];
    if(!(fp=fopen("/proc/net/dev", "r")))
    {
	return 0;
    }
    for(i=0; i<2; i++)
    {
	if(fgets(ibuf, sizeof(ibuf), fp)==NULL)
	{
	    fclose(fp);
	    return 0;
	}
    }
    while(f)
    {
	if(fgets(ibuf, sizeof(ibuf), fp)==NULL)
	{
	    if(feof(fp))
	    f = 0;
	    fclose(fp);
	    f = 0;
	}
	if(!(colon=strchr(ibuf, ':')))
	{
	    return 0;
	}
	*colon=' ';
	sscanf(ibuf, "%s", ibuf2);
	if (!strcmp(ibuf2, iface))
	{
	    sscanf (ibuf,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",&device[0],&rbytes[0],&rpackets[0],&rerrors[0],&rdrop[0],&rfifo[0],&rframe[0],&rcompressed[0],&rmulticast[0],&tbytes[0],&tpackets[0],&terrors[0],&tdrop[0],&tfifo[0],&tcolls[0],&tcarrier[0],&tcompressed[0]);
	    printf("\t%s packets input, %s bytes, %s input errors\n",rpackets,rbytes,rerrors);
	    printf("\t%s input drops, %s overruns, %s frame %s compressed, %s multicast\n",rdrop,rfifo,rframe,rcompressed,rmulticast);
	    printf("\t%s packets output, %s bytes, %s output errors\n",&tpackets[0],&tbytes[0],&terrors[0]);
	    printf("\t%s errors, %s output drops, %s overruns, %s collisions, %s carrier, %s compressed\n",&terrors[0],&tdrop[0],&tfifo[0],&tcolls[0],&tcarrier[0],&tcompressed[0]);
	}
    }
    return 0;
}     

    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include "../lib/ext.h"

#define inaddrr( x ) ( *( struct in_addr * )&ifr->x[ sizeof( sa.sin_port ) ] )
#define IFRSIZE      ( ( int )( size * sizeof( struct ifreq ) ) )

int ethshowall(void)
{
    //unsigned char *    u;
    int  sockfd, size = 1;
    int i;
    struct ifreq *     ifr;
    struct ifconf      ifc;
    //struct sockaddr_in sa;
    int up=0;
    int running=0;
    int noarp=0;
    int multi=0;
    int bcast=0;

    if ( 0 > ( sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_IP ) ) )
    {
        fprintf( stderr, "Cannot open socket.\n" );
        return (-1);
    }
    ifc.ifc_req = NULL;
    do
    {
        ++size;
        /* realloc buffer size until no overflow occurs */
        if ( NULL == ( ifc.ifc_req = realloc( ifc.ifc_req, IFRSIZE ) ) )
        {
            fprintf( stderr, "Out of memory.\n" );
            return(-1);
        }
        ifc.ifc_len = IFRSIZE;
        if ( ioctl( sockfd, SIOCGIFCONF, &ifc ) )
        {
            perror( "ioctl SIOCFIFCONF" );
            return (-1);
        }
    } while ( IFRSIZE <= ifc.ifc_len );
    ifr = ifc.ifc_req;
    for ( ; ( char * )ifr < ( char * )ifc.ifc_req + ifc.ifc_len; ++ifr )
    {
        if ( ifr->ifr_addr.sa_data == ( ifr + 1 )->ifr_addr.sa_data )
        {
            /* duplicate, skip it */
            continue;
        }
        if ( ioctl( sockfd, SIOCGIFFLAGS, ifr ) )
        {
            /* failed to get flags, skip it */
            continue;
        }
	if(!(ifr->ifr_flags & IFF_LOOPBACK)){
	if (ioctl(sockfd, SIOCGIFFLAGS, ifr) == 0) 
	{
	    /* flags */
	    if (ifr->ifr_flags & IFF_UP) up = 1; else up=0; 
	    if (ifr->ifr_flags & IFF_RUNNING) running = 1; else running=0;
	    if (ifr->ifr_flags & IFF_NOARP) noarp = 1; else noarp=0;
	    if (ifr->ifr_flags & IFF_MULTICAST) multi = 1; else multi=0;
	    if (ifr->ifr_flags & IFF_BROADCAST) bcast = 1; else bcast=0;
	}
	printf( "Ethernet %s is ", ifr->ifr_name);
	printf("%s, line protocol is %s\n", up ? "up": "down", running ? "up" : "down");
	if (ioctl(sockfd, SIOCGIFADDR, ifr) == 0) 
	{
	    printf("\tInternet address is ");
	    for( i=2; i<6; i++)
    	    {
                unsigned char val = (unsigned char)ifr->ifr_addr.sa_data[i];
    		printf( "%d%s", val, i==5?" ":".");
    	    }
	}
/*	if (ioctl(sockfd,SIOCGIFBRDADDR, ifr) >= 0) 
	{
	    printf( "\tBcast ");
	    for( i=2; i<6; i++)
	    {
    		unsigned char val = (unsigned char)ifr->ifr_addr.sa_data[i];
    		printf( "%d%s", val, i==5?" ":".");
    	    }
	    //printf( "\n");
	}
*/	if (ioctl(sockfd,SIOCGIFNETMASK, ifr) >= 0) 
	{
	    printf( ", subnet mask is ");
	    for( i=2; i<6; i++)
	    {
        	unsigned char val = (unsigned char)ifr->ifr_addr.sa_data[i];
    		printf( "%d%s", val, i==5?" ":".");
    	    }
	    printf( "\n");
	}
/*	printf( noarp ? "ARP, " : "No ARP, ");
	printf( bcast ? "Broadcast, " : "no Broadcast, ");
	printf( multi ? "Multicast" : "no Multicast");
	printf("\n");
*/
	if (ioctl(sockfd,SIOCGIFHWADDR, ifr) >= 0) 
	{
	    printf( "\tHardware address is ");
	    printf("%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
	    (unsigned char)ifr->ifr_hwaddr.sa_data[0],
	    (unsigned char)ifr->ifr_hwaddr.sa_data[1],
	    (unsigned char)ifr->ifr_hwaddr.sa_data[2],
	    (unsigned char)ifr->ifr_hwaddr.sa_data[3],
	    (unsigned char)ifr->ifr_hwaddr.sa_data[4],
	    (unsigned char)ifr->ifr_hwaddr.sa_data[5]);
	}    
	if (ioctl(sockfd, SIOCGIFMAP, ifr) >= 0)
        {
            printf( ", irq %d, ", ifr->ifr_map.irq);
            printf( "base addr 0x%x\n",ifr->ifr_map.base_addr);
        //    printf( "port %d, ", ifr->ifr_map.port);
        //    printf( "dma %x, ", ifr->ifr_map.dma);
    	//    printf( "memory %lx-%lx\n", ifr->ifr_map.mem_start,ifr->ifr_map.mem_end);
        }
	if (ioctl(sockfd,SIOCGIFMTU, ifr) >= 0) 
	{
	    printf( "\tMTU %d bytes, ",ifr->ifr_mtu );
	}
	if (ioctl(sockfd,SIOCGIFMETRIC, ifr) >= 0) 
	{
	    printf( "metric %u, ",ifr->ifr_metric ? ifr->ifr_metric : 1 );
	}
/*	if (ioctl(sockfd,SIOCGIFFLAGS, ifr) >= 0) 
	{
	    printf( "flags %d, ",ifr->ifr_flags );
	}
*/
	if (ioctl(sockfd,SIOCGIFTXQLEN, ifr) >= 0) 
	{
	    printf( "txqueuelen %d\n",ifr->ifr_qlen );
	}
	else printf("%% Invalid interface\n");    
       infoall(ifr->ifr_name);
      }}
      close( sockfd );
    return 0;
}

int infoall(char *iface)
{
    int f = 1;
    int i;
    FILE *fp;
    char *colon = NULL, device[64],
    rbytes[64],rpackets[64],rerrors[64],rdrop[64],rfifo[64],rframe[64],rcompressed[64],
    rmulticast[64],
    tbytes[64],tpackets[64],terrors[64],tdrop[64],tfifo[64],tcolls[64],tcarrier[64],tcompressed[64];
	  
    char buf[1024];
    char buf2[20];
    if(!(fp=fopen("/proc/net/dev", "r")))
    {
	return 0;
    }
    for(i=0; i<2; i++)
    {
	if(fgets(buf, sizeof(buf), fp)==NULL)
	{
	    fclose(fp);
	    return 0;
	}
    }
    while(f)
    {
	if(fgets(buf, sizeof(buf), fp)==NULL)
	{
	    if(feof(fp))
	    f = 0;
	    fclose(fp);
	    f = 0;
	}
	if(!(colon=strchr(buf, ':')))
	{
	    return 0;
	}
	*colon=' ';
	sscanf(buf, "%s", buf2);
	if (!strcmp(buf2, iface))
	{
	    sscanf (buf,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",&device[0],&rbytes[0],&rpackets[0],&rerrors[0],&rdrop[0],&rfifo[0],&rframe[0],&rcompressed[0],&rmulticast[0],&tbytes[0],&tpackets[0],&terrors[0],&tdrop[0],&tfifo[0],&tcolls[0],&tcarrier[0],&tcompressed[0]);
	    printf("\t%s packets input, %s bytes, %s input errors\n",rpackets,rbytes,rerrors);
	    printf("\t%s input drops, %s overruns, %s frame %s compressed, %s multicast\n",rdrop,rfifo,rframe,rcompressed,rmulticast);
	    printf("\t%s packets output, %s bytes, %s output errors\n",&tpackets[0],&tbytes[0],&terrors[0]);
	    printf("\t%s output drops, %s overruns, %s collisions, %s carrier, %s compressed\n",&tdrop[0],&tfifo[0],&tcolls[0],&tcarrier[0],&tcompressed[0]);
	}
    }
return 0;
}     


