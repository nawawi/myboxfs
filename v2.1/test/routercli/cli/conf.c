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
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/route.h>
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
#include <net/if_arp.h>
#define inaddrr( x ) ( *( struct in_addr * )&ifr->x[ sizeof( sa.sin_port ) ] )
#define IFRSIZE      ( ( int )( size * sizeof( struct ifreq ) ) )
#include "../lib/ext.h"
char enpass[50];
char nsname[50];
char host[40];
char **chargv;
char banner[100];

int cli_show_run(int opt)
{
    char buf[BUFSIZ];
    int  sockfd, size = 1;
    int i,n;
    struct ifreq *     ifr;
    struct ifconf      ifc;
    char buff[256];
    int  nl = 0 ;
    struct in_addr dest;
    struct in_addr gw;
    struct in_addr mask;
    int flgs, ref, use, metric;
    int via;
    int viagw;
    unsigned long int d,g,m;
    char sdest[16], sgw[36];
    char line[MAX_LINE_LEN + 1] ;
    FILE *fp = NULL;
    FILE *dns = NULL;  
    FILE *config_fp = NULL;
    FILE *pidfile = NULL;
    FILE *aclist = NULL;
    char c;
    if (opt==1)
    {
	config_fp = fopen( "/etc/routercli/cli.conf", "w" ) ;
	if (config_fp==NULL)
	{
	    fprintf(stderr,"%% Error write config\n");
	    return (-1);
	}
    }
    gethostname(host,sizeof(host));
    if (opt==1)
    {
	fprintf(config_fp,"version %s\n",VERSION);
	fprintf(config_fp,"%s",ZNAK);
	fprintf(config_fp,"hostname %s\n",host);
	fprintf(config_fp,"%s",ZNAK); 
    }
    else 
    {
	printf ("Building configuration...\r\n");
	printf ("Current configuration:\r\n");
	printf("version %s\n",VERSION);
	printf("%s",ZNAK);
	printf("hostname %s\n",host);
	printf("%s",ZNAK); 
    }
    if (banner)
    {
     
	if (opt==1)
    	    fprintf(config_fp,"banner login %s\n!\n",banner);
	else 
    	    printf("banner login %s\n!\n",banner);
    }
    if (enpass)
    { 
	if (opt==1)
    	    fprintf(config_fp,"enable secret %s\n!\n",enpass);
	else 
    	    printf("enable secret %s\n!\n",enpass);
    }
    dns = fopen( "/etc/resolv.conf", "r" ); 
    if (dns==NULL) { 
	fprintf (stderr,"%% Error getting nameservers\r\n");
    }
    else 
    {      
	while ( fgets( line, MAX_LINE_LEN, dns ) != NULL ) 
	{
	    if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;
	    if (makeargv(line, BLANK_STRING, &chargv) > 0) 
	    {
    		if(chargv[0]!= NULL) 
		{ 
		    
		if (strcmp(chargv[0],"search")==0 && chargv[1])
			if (opt==1)
			    fprintf (config_fp,"ip domain-name %s\n",chargv[1]);
			else
			    printf ("ip domaine-name %s\n",chargv[1]);	
	
		else if (strcmp(chargv[0],"nameserver")==0 && chargv[1])	{
			if (opt==1) {
			    fprintf (config_fp,"ip name-server %s\n",chargv[1]);
			}
			else	{
			    printf ("ip name-server %s\n",chargv[1]);	
			}
		     }
		}
	    	
	    }
	}    	
    }
    fclose(dns);
    if (opt==1)
	fprintf (config_fp,"%s",ZNAK);
    else
	printf ("%s",ZNAK);
	 
// ifaces    
    if ( 0 > ( sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_IP))) {
        fprintf( stderr, "Cannot open socket.\n" );
        return(-1);
    }
    ifc.ifc_req = NULL;
    do
    {
        ++size;
        if ( NULL == ( ifc.ifc_req = realloc( ifc.ifc_req, IFRSIZE))) {
            fprintf( stderr, "Out of memory.\n" );
            return(-1);
        }
        ifc.ifc_len = IFRSIZE;
        if ( ioctl( sockfd, SIOCGIFCONF, &ifc )) {
            perror( "ioctl SIOCFIFCONF" );
            return(-1);
        }
    } while ( IFRSIZE <= ifc.ifc_len );
    ifr = ifc.ifc_req;
    for ( ; ( char * )ifr < ( char * )ifc.ifc_req + ifc.ifc_len; ++ifr )
    {
        if ( ifr->ifr_addr.sa_data == ( ifr + 1 )->ifr_addr.sa_data) {
            continue;
        }
        if ( ioctl( sockfd, SIOCGIFFLAGS, ifr )) {
            continue;
        }
	if(!(ifr->ifr_flags & IFF_LOOPBACK)) {
	    if (opt==1)
		fprintf(config_fp, "interface %s\n", ifr->ifr_name);
	    else
		printf( "interface %s\n", ifr->ifr_name);
	    if (ioctl(sockfd, SIOCGIFADDR, ifr) == 0) {
		if (opt==1)	
		    fprintf(config_fp," ip address ");
		else    
		    printf(" ip address ");
		for( i=2; i<6; i++)
    		{
            	    unsigned char val = (unsigned char)ifr->ifr_addr.sa_data[i];
    		    if (opt==1)
			fprintf(config_fp, "%d%s", val, i==5?" ":".");
		    else
			printf( "%d%s", val, i==5?" ":".");
		}
	    }
	    if (ioctl(sockfd,SIOCGIFNETMASK, ifr) >= 0) {
		for( i=2; i<6; i++)
		{
        	    unsigned char val = (unsigned char)ifr->ifr_addr.sa_data[i];
    		    if (opt==1)
			fprintf(config_fp, "%d%s", val, i==5?" ":".");
		    else
			printf( "%d%s", val, i==5?" ":".");
		}
		if (opt==1)
		    fprintf(config_fp,"\n");
		else
		    printf ("\n");
	    }
	}
    }

    if (opt==1)
	fprintf (config_fp,"%s",ZNAK);
    else
	printf ("%s",ZNAK);
    close( sockfd );
    // route
    fp = xfopen("/proc/net/route", "r");
    while( fgets(buff, sizeof(buff), fp) != NULL ) {
	if(nl) {
	    int ifl = 0;
	    while(buff[ifl]!=' ' && buff[ifl]!='\t' && buff[ifl]!='\0')
	    ifl++;
	    buff[ifl]=0;
	    if(sscanf(buff+ifl+1, "%lx%lx%d%d%d%d%lx", &d, &g, &flgs, &ref, &use, &metric, &m)!=7) 
		printf( "Unsuported kernel route format\n");
	    if(nl==1) {
		via=0;
		viagw=0;
	    }
	    dest.s_addr = d;
	    gw.s_addr   = g;
	    mask.s_addr = m;
	    if (dest.s_addr==0) {	
		via=0;
	    } else {
    		strcpy(sdest,inet_ntoa(dest)); via=1;
	    }
	    if (gw.s_addr==0) {
		viagw=0;
	    } else {
		strcpy(sgw,inet_ntoa(gw)); viagw=1;
	    }
	    if (!via) {
		if (opt==1)
		{
		    fprintf (config_fp,"%s",ZNAK);
		    fprintf(config_fp,"ip default-gateway %s\n",sgw);
		} else {
		    printf ("%s",ZNAK);
		    printf("ip default-gateway %s\n",sgw);
		}
	    } else if (!viagw) {
		if (opt==1)
		    fprintf(config_fp,"ip route %s %s %s\n",sdest, inet_ntoa(mask),cli_chomp(sgw));
		else
		    printf("ip route %s %s %s\n",sdest, inet_ntoa(mask),cli_chomp(sgw));
	    }	
	}
	nl++;
    }
    fclose(fp);
    if (opt==1)
	fprintf(config_fp,"%s",ZNAK);
    else
	printf("%s",ZNAK); 
    
    pidfile = fopen( "/var/run/ripd.pid", "r" );
    if (pidfile==NULL)
    {
	if (opt==1)
	    fprintf(config_fp,"%s",ZNAK);
	else
	    printf("%s",ZNAK); 
    }
    else
    {
	if (opt==1)
	    fprintf(config_fp,"router rip\n!");
	else
	    printf("router rip\n");
	fclose(pidfile);
    }
    aclist = fopen( "/etc/routercli/aclist", "r" ); 
    if (aclist==NULL) { 
	if (opt==1)
	{
	    fprintf(config_fp,"%s",ZNAK);
	    fprintf(config_fp,"end\n");
	    fclose(config_fp);
	} else {
	    printf("%s",ZNAK);
	    printf("end\n");
	}
	return(0);
    }
    else 
    {
	c = fgetc(aclist);
	while(c!=EOF)
	{
	    if (opt==1)
	    {	
		while ((n = fread(buf, sizeof(char), BUFSIZ, aclist)) > 0)
		        fwrite(buf, sizeof(char), n, config_fp);
	    }
	    else	
		putchar(c);
	    c =fgetc(aclist);
	}
    }
    fclose(aclist);
    
    if (opt==1)
    {
	fprintf(config_fp,"%s",ZNAK);
	fprintf(config_fp,"end\n");
	fclose(config_fp);
    } else {
	printf("%s",ZNAK);
	printf("end\n");
    }	
    return 0;
}
