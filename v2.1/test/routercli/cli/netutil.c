/*
* Copyright (c) 2004 Alexander Eremin <xyzyx@rambler.ru>
*
* Utility routines.
*
* create raw socket for icmp protocol test permision
* and drop root privilegies if running setuid
*
*/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lib/ext.h"

int create_icmp_socket(void)
{
	struct protoent *proto;
	int sock;

	proto = getprotobyname("icmp");
	/* if getprotobyname failed, just silently force
	 * proto->p_proto to have the correct value for "icmp" */
	if ((sock = socket(AF_INET, SOCK_RAW,
			(proto ? proto->p_proto : 1))) < 0) {        /* 1 == ICMP */
		if (errno == EPERM)
			{
			    printf("%% Permissions denied (are you root?)");
			    exit (1);
			}
		else
			{
			    printf("%% Cant create raw socket");
			    exit (1);
			}    
	}

	/* drop root privs if running setuid */
	setuid(getuid());

	return sock;
}

int INET_resolve(const char *name, struct sockaddr_in *s_in, int hostfirst)
{
	struct hostent *hp;
	struct netent *np;

	/* Grmpf. -FvK */
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;

	/* Default is special, meaning 0.0.0.0. */
	if (!strcmp(name, "default")) {
		s_in->sin_addr.s_addr = INADDR_ANY;
		return (1);
	}
	/* Look to see if it's a dotted quad. */
	if (inet_aton(name, &s_in->sin_addr)) {
		return 0;
	}
	/* If we expect this to be a hostname, try hostname database first */
	if (hostfirst && (hp = gethostbyname(name)) != (struct hostent *) NULL) {
		memcpy((char *) &s_in->sin_addr, (char *) hp->h_addr_list[0],
			   sizeof(struct in_addr));
		return 0;
	}
	/* Try the NETWORKS database to see if this is a known network. */
	if ((np = getnetbyname(name)) != (struct netent *) NULL) {
		s_in->sin_addr.s_addr = htonl(np->n_net);
		return 1;
	}
	if (hostfirst) {
		/* Don't try again */
		errno = h_errno;
		return -1;
	}

	if ((hp = gethostbyname(name)) == (struct hostent *) NULL) {
		errno = h_errno;
		return -1;
	}
	memcpy((char *) &s_in->sin_addr, (char *) hp->h_addr_list[0],
		   sizeof(struct in_addr));

	return 0;
}

/* cache */
struct addr {
	struct sockaddr_in addr;
	char *name;
	int host;
	struct addr *next;
};

static struct addr *INET_nn = NULL;	/* addr-to-name cache           */

/* numeric: & 0x8000: default instead of *,
 *          & 0x4000: host instead of net,
 *          & 0x0fff: don't resolve
 */
int INET_rresolve(char *name, size_t len, struct sockaddr_in *s_in,
				  int numeric, unsigned int netmask)
{
	struct hostent *ent;
	struct netent *np;
	struct addr *pn;
	unsigned long ad, host_ad;
	int host = 0;

	/* Grmpf. -FvK */
	if (s_in->sin_family != AF_INET) {
		errno = EAFNOSUPPORT;
		return (-1);
	}
	ad = (unsigned long) s_in->sin_addr.s_addr;
	if (ad == INADDR_ANY) {
		if ((numeric & 0x0FFF) == 0) {
			if (numeric & 0x8000)
				safe_strncpy(name,"default", len);
			else
				safe_strncpy(name, "*", len);
			return (0);
		}
	}
	if (numeric & 0x0FFF) {
		safe_strncpy(name, inet_ntoa(s_in->sin_addr), len);
		return (0);
	}

	if ((ad & (~netmask)) != 0 || (numeric & 0x4000))
		host = 1;
#if 0
	INET_nn = NULL;
#endif
	pn = INET_nn;
	while (pn != NULL) {
		if (pn->addr.sin_addr.s_addr == ad && pn->host == host) {
			safe_strncpy(name, pn->name, len);
			return (0);
		}
		pn = pn->next;
	}

	host_ad = ntohl(ad);
	np = NULL;
	ent = NULL;
	if (host) {
		ent = gethostbyaddr((char *) &ad, 4, AF_INET);
		if (ent != NULL) {
			safe_strncpy(name, ent->h_name, len);
		}
	} else {
		np = getnetbyaddr(host_ad, AF_INET);
		if (np != NULL) {
			safe_strncpy(name, np->n_name, len);
		}
	}
	if ((ent == NULL) && (np == NULL)) {
		safe_strncpy(name, inet_ntoa(s_in->sin_addr), len);
	}
	pn = (struct addr *) xmalloc(sizeof(struct addr));
	pn->addr = *s_in;
	pn->next = INET_nn;
	pn->host = host;
	pn->name = strdup(name);
	INET_nn = pn;

	return (0);
}

