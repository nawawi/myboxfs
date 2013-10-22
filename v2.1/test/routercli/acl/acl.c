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

#include "include/libiptc/libiptc.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/netfilter.h> 
#include "include/libiptc/iptables.h" 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int acl_stand(int argc, char **argv) 
{
        long result = 0;
        iptc_handle_t handle = NULL;
        struct ipt_standard_target target;
        struct ipt_entry *chain_entry;
        ipt_chainlabel labelit; 
        long size=0; 
        chain_entry = NULL; 
        if (strcmp(argv[0],"deny")==0)
	    strncpy(&target.target.u.user.name[0], "DROP", 30);  
        else if (strcmp(argv[0],"permit")==0)
	    strncpy(&target.target.u.user.name[0], "ACCEPT", 30); 
        target.target.u.user.target_size=sizeof(target); 
        chain_entry = malloc(sizeof(struct ipt_entry) + sizeof(target));
        memset(chain_entry, 0, sizeof(chain_entry));   
        if (strcmp(argv[4],"in")==0)
	{   
	    if (argv[1])
	        chain_entry->ip.src.s_addr=inet_addr(argv[1]); 
	    if (argv[2])
		chain_entry->ip.smsk.s_addr=inet_addr(argv[2]); 
	    else 
		chain_entry->ip.smsk.s_addr=inet_addr(" ");
	    chain_entry->ip.dst.s_addr=inet_addr("any");
	    chain_entry->ip.dmsk.s_addr=inet_addr("0");
	}
	else if (strcmp(argv[4],"out")==0)
	{    
	    chain_entry->ip.src.s_addr=inet_addr("any");
	    chain_entry->ip.smsk.s_addr=inet_addr("0");
	    if (argv[1])
		chain_entry->ip.dst.s_addr=inet_addr(argv[1]);
	    if (argv[2])
		chain_entry->ip.dmsk.s_addr=inet_addr(argv[2]);
	    else 
		chain_entry->ip.dmsk.s_addr=inet_addr(" ");
	}
	chain_entry->ip.proto=6; /* any protocol */ 
	sprintf(chain_entry->ip.iniface,argv[3]);
	sprintf(chain_entry->ip.outiface,argv[3]);
	size=sizeof(struct ipt_entry);
	chain_entry->target_offset=size;
	chain_entry->next_offset= size + sizeof(target); 
	memcpy(chain_entry->elems, &target, sizeof(target)); 
	handle = iptc_init("filter");   
	if (handle == NULL)
	{
    	    printf("%% Error initializing: %s\n", iptc_strerror(errno));
    	    return(-1);
	}
	else
    	    ;
	//    printf("table exists\n");
	if (strcmp(argv[5],"vty")==0)
	    result = iptc_is_chain("INPUT", handle);
	else 
	    result = iptc_is_chain("FORWARD", handle);
	if(result)                              
    	//    printf("chain exists\n");
	    ;
	else
	{
    	//    printf("error: chain does not exist!\n");
    	    return (-1);
	} 
	if (strcmp(argv[5],"vty")==0)
	    strncpy(labelit, "INPUT", 30); 
	else 
	    strncpy(labelit, "FORWARD", 30);
	result = iptc_append_entry(labelit, chain_entry, &handle);
	if(result == 0)
	    ;    
	//    printf("append error: %s\n", iptc_strerror(errno)); 
	result=iptc_commit(&handle);
	if(result == 0)
	    //printf("commit error: %s\n", iptc_strerror(errno));
	    ;
	else
	//    printf("appended new rule to block successfully\n"); 
	    ;
	free(chain_entry);
	return 0;
} 

int noacl(void)
{
    long result = 0;
    iptc_handle_t h;
    const char *chain = NULL;
    const char *tablename = "filter";
    h = iptc_init(tablename);
    if ( !h )   {
    printf("%% Error initializing: %s\n", iptc_strerror(errno));
    return(-1);
    }
    for (chain = iptc_first_chain(&h); chain; chain = iptc_next_chain(&h))  
    {
	//printf("%s\n", chain);
        result=iptc_flush_entries(chain, &h);
    }
    result=iptc_commit(&h);
    return 0;
}
