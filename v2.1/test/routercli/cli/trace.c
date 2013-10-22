//#undef CONFIG_FEATURE_TRACEROUTE_VERBOSE
#define CONFIG_FEATURE_TRACEROUTE_VERBOSE
#undef CONFIG_FEATURE_TRACEROUTE_SO_DEBUG   /* not in documentation man */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <endian.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include "../lib/ext.h"


#define MAXPACKET       65535   /* max ip packet size */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  64
#endif

/*
 * format of a (udp) probe packet.
 */
struct opacket {
	struct ip ip;
	struct udphdr udp;
	u_char seq;             /* sequence number of this packet */
	u_char ttl;             /* ttl packet left with */
	struct timeval tv;      /* time packet left */
};

/*
 * Definitions for internet protocol version 4.
 * Per RFC 791, September 1981.
 */
#define IPVERSION       4



static u_char  packet[512];            /* last inbound (icmp) packet */
static struct opacket  *outpacket;     /* last output (udp) packet */

static int s;                          /* receive (icmp) socket file descriptor */
static int sndsock;                    /* send (udp) socket file descriptor */

static struct sockaddr whereto;        /* Who to try to reach */
static int datalen;                    /* How much data */

static char *hostname;

static int max_ttl = 30;
static u_short ident;
static u_short port = 32768+666;       /* start udp dest port # for probe packets */

#ifdef CONFIG_FEATURE_TRACEROUTE_VERBOSE
static int verbose;
#endif
static int waittime = 5;               /* time to wait for response (in seconds) */
static int nflag;                      /* print addresses numerically */

/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give
 * numeric value, otherwise try for symbolic name.
 */

static inline void
inetname(struct sockaddr_in *from)
{
	char *cp;
	static char domain[MAXHOSTNAMELEN + 1];
	char name[MAXHOSTNAMELEN + 1];
	static int first = 1;
	const char *ina;

	if (first && !nflag) {
		first = 0;
		if (getdomainname(domain, MAXHOSTNAMELEN) != 0)
			domain[0] = 0;
	}
	cp = 0;
	if (!nflag && from->sin_addr.s_addr != INADDR_ANY) {
		if(INET_rresolve(name, sizeof(name), from, 0x4000, 0xffffffff) >= 0) {
			if ((cp = strchr(name, '.')) &&
			    !strcmp(cp + 1, domain))
				*cp = 0;
			cp = (char *)name;
		}
	}
	ina = inet_ntoa(from->sin_addr);
	if (nflag)
		printf(" %s", ina);
	else
		printf(" %s (%s)", (cp ? cp : ina), ina);
}

static inline void
print(u_char *buf, int cc, struct sockaddr_in *from)
{
	struct ip *ip;
	int hlen;

	ip = (struct ip *) buf;
	hlen = ip->ip_hl << 2;
	cc -= hlen;

	inetname(from);
#ifdef CONFIG_FEATURE_TRACEROUTE_VERBOSE
	if (verbose)
		printf (" %d bytes to %s", cc, inet_ntoa (ip->ip_dst));
#endif
}

static inline double
deltaT(struct timeval *t1p, struct timeval *t2p)
{
	double dt;

	dt = (double)(t2p->tv_sec - t1p->tv_sec) * 1000.0 +
	     (double)(t2p->tv_usec - t1p->tv_usec) / 1000.0;
	return (dt);
}

static inline int
wait_for_reply(int sock, struct sockaddr_in *from, int reset_timer)
{
	fd_set fds;
	static struct timeval wait;
	int cc = 0;
	int fromlen = sizeof (*from);

	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	if (reset_timer) {
		/*
		 * traceroute could hang if someone else has a ping
		 * running and our ICMP reply gets dropped but we don't
		 * realize it because we keep waking up to handle those
		 * other ICMP packets that keep coming in.  To fix this,
		 * "reset_timer" will only be true if the last packet that
		 * came in was for us or if this is the first time we're
		 * waiting for a reply since sending out a probe.  Note
		 * that this takes advantage of the select() feature on
		 * Linux where the remaining timeout is written to the
		 * struct timeval area.
		 */
		wait.tv_sec = waittime;
		wait.tv_usec = 0;
	}

	if (select(sock+1, &fds, (fd_set *)0, (fd_set *)0, &wait) > 0)
		cc=recvfrom(s, (char *)packet, sizeof(packet), 0,
			    (struct sockaddr *)from, &fromlen);

	return(cc);
}

#ifdef CONFIG_FEATURE_TRACEROUTE_VERBOSE
/*
 * Convert an ICMP "type" field to a printable string.
 */
static inline const char *
pr_type(u_char t)
{
	static const char * const ttab[] = {
	"Echo Reply",   "ICMP 1",       "ICMP 2",       "Dest Unreachable",
	"Source Quench", "Redirect",    "ICMP 6",       "ICMP 7",
	"Echo",         "ICMP 9",       "ICMP 10",      "Time Exceeded",
	"Param Problem", "Timestamp",   "Timestamp Reply", "Info Request",
	"Info Reply"
	};

	if(t > 16)
		return("OUT-OF-RANGE");

	return(ttab[t]);
}
#endif

static inline int
packet_ok(u_char *buf, int cc, struct sockaddr_in *from, int seq)
{
	struct icmp *icp;
	u_char type, code;
	int hlen;
	struct ip *ip;

	ip = (struct ip *) buf;
	hlen = ip->ip_hl << 2;
	if (cc < hlen + ICMP_MINLEN) {
#ifdef CONFIG_FEATURE_TRACEROUTE_VERBOSE
		if (verbose)
			printf("packet too short (%d bytes) from %s\n", cc,
				inet_ntoa(from->sin_addr));
#endif
		return (0);
	}
	cc -= hlen;
	icp = (struct icmp *)(buf + hlen);
	type = icp->icmp_type; code = icp->icmp_code;
	if ((type == ICMP_TIMXCEED && code == ICMP_TIMXCEED_INTRANS) ||
	    type == ICMP_UNREACH) {
		struct ip *hip;
		struct udphdr *up;

		hip = &icp->icmp_ip;
		hlen = hip->ip_hl << 2;
		up = (struct udphdr *)((u_char *)hip + hlen);
		if (hlen + 12 <= cc && hip->ip_p == IPPROTO_UDP &&
		    up->source == htons(ident) &&
		    up->dest == htons(port+seq))
			return (type == ICMP_TIMXCEED? -1 : code+1);
	}
#ifdef CONFIG_FEATURE_TRACEROUTE_VERBOSE
	if (verbose) {
		int i;
		u_long *lp = (u_long *)&icp->icmp_ip;

		printf("\n%d bytes from %s to %s: icmp type %d (%s) code %d\n",
			cc, inet_ntoa(from->sin_addr), inet_ntoa(ip->ip_dst),
			type, pr_type(type), icp->icmp_code);
		for (i = 4; i < cc ; i += sizeof(long))
			printf("%2d: x%8.8lx\n", i, *lp++);
	}
#endif
	return(0);
}

static void             /* not inline */
send_probe(int seq, int ttl)
{
	struct opacket *op = outpacket;
	struct ip *ip = &op->ip;
	struct udphdr *up = &op->udp;
	int i;
	struct timezone tz;

	ip->ip_off = 0;
	ip->ip_hl = sizeof(*ip) >> 2;
	ip->ip_p = IPPROTO_UDP;
	ip->ip_len = datalen;
	ip->ip_ttl = ttl;
	ip->ip_v = IPVERSION;
	ip->ip_id = htons(ident+seq);

	up->source = htons(ident);
	up->dest = htons(port+seq);
	up->len = htons((u_short)(datalen - sizeof(struct ip)));
	up->check = 0;

	op->seq = seq;
	op->ttl = ttl;
	(void) gettimeofday(&op->tv, &tz);

	i = sendto(sndsock, (char *)outpacket, datalen, 0, &whereto,
		   sizeof(struct sockaddr));
	if (i < 0 || i != datalen)  {
		if (i<0)
			perror("sendto");
		printf("traceroute: wrote %s %d chars, ret=%d\n", hostname,
			datalen, i);
		(void) fflush(stdout);
	}
}


int
trace_main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind;
	struct hostent *hp;
	struct sockaddr_in from, *to;
	int ch, i, on, probe, seq, tos, ttl;

	int options = 0;                /* socket options */
	char *source = 0;
	int nprobes = 3;

	on = 1;
	seq = tos = 0;
	to = (struct sockaddr_in *)&whereto;
	while ((ch = getopt(argc, argv, "dm:np:q:rs:t:w:v")) != EOF)
		switch(ch) {
		case 'd':
#ifdef CONFIG_FEATURE_TRACEROUTE_SO_DEBUG
			options |= SO_DEBUG;
#endif
			break;
		case 'm':
			max_ttl = atoi(optarg);
			if (max_ttl <= 1)
				{
				    printf("%% max ttl must be >1.\n");
				    exit (1);
				}
			break;
		case 'n':
			nflag++;
			break;
		case 'p':
			port = atoi(optarg);
			if (port < 1)
				{
				    printf("%% port must be >0.\n");
				    exit(1);
				}    
			break;
		case 'q':
			nprobes = atoi(optarg);
			if (nprobes < 1)
				{
				    printf("%% nprobes must be >0.\n");
				    exit (1);
				}
			break;
		case 'r':
			options |= SO_DONTROUTE;
			break;
		case 's':
			/*
			 * set the ip source address of the outbound
			 * probe (e.g., on a multi-homed host).
			 */
			source = optarg;
			break;
		case 't':
			tos = atoi(optarg);
			if (tos < 0 || tos > 255)
				{
				    printf("%% tos must be 0 to 255.\n");
				    exit (1);
				}
			break;
		case 'v':
#ifdef CONFIG_FEATURE_TRACEROUTE_VERBOSE
			verbose++;
#endif
			break;
		case 'w':
			waittime = atoi(optarg);
			if (waittime <= 1)
				{
				    printf("%% wait must be >1 sec.\n");
				    exit (1);
				}    
			break;
		default:
			printf("%% Incomplete command\r\n");
		}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		printf("%% Incomplete command\r\n");

	setlinebuf (stdout);

	memset(&whereto, 0, sizeof(struct sockaddr));
	hp = xgethostbyname(*argv);
			to->sin_family = hp->h_addrtype;
	memcpy(&to->sin_addr, hp->h_addr, hp->h_length);
			hostname = (char *)hp->h_name;
	if (*++argv)
		datalen = atoi(*argv);
	if (datalen < 0 || datalen >= MAXPACKET - sizeof(struct opacket))
		{
		    printf("%% packet size must be 0 <= s < %d.\n",
		    MAXPACKET - sizeof(struct opacket));
		    exit (1);
		}
	datalen += sizeof(struct opacket);
	outpacket = (struct opacket *)xmalloc((unsigned)datalen);
	memset(outpacket, 0, datalen);
	outpacket->ip.ip_dst = to->sin_addr;
	outpacket->ip.ip_tos = tos;
	outpacket->ip.ip_v = IPVERSION;
	outpacket->ip.ip_id = 0;

	ident = (getpid() & 0xffff) | 0x8000;

	if ((sndsock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		{
		    printf("%% error create raw socket\n");
		    exit (1);
		}
	s = create_icmp_socket();

#ifdef CONFIG_FEATURE_TRACEROUTE_SO_DEBUG
	if (options & SO_DEBUG)
		(void) setsockopt(s, SOL_SOCKET, SO_DEBUG,
				  (char *)&on, sizeof(on));
#endif
	if (options & SO_DONTROUTE)
		(void) setsockopt(s, SOL_SOCKET, SO_DONTROUTE,
				  (char *)&on, sizeof(on));
#ifdef SO_SNDBUF
	if (setsockopt(sndsock, SOL_SOCKET, SO_SNDBUF, (char *)&datalen,
		       sizeof(datalen)) < 0)
		{
		    printf("%% error SO_SNDBUF\n");
		    exit (1);
		}
#endif
#ifdef IP_HDRINCL
	if (setsockopt(sndsock, IPPROTO_IP, IP_HDRINCL, (char *)&on,
		       sizeof(on)) < 0)
		{
		    printf("%% error IP_HDRINCL\n");
		    exit (1);
		}
		
#endif
#ifdef CONFIG_FEATURE_TRACEROUTE_SO_DEBUG
	if (options & SO_DEBUG)
		(void) setsockopt(sndsock, SOL_SOCKET, SO_DEBUG,
				  (char *)&on, sizeof(on));
#endif
	if (options & SO_DONTROUTE)
		(void) setsockopt(sndsock, SOL_SOCKET, SO_DONTROUTE,
				  (char *)&on, sizeof(on));

	if (source) {
		memset(&from, 0, sizeof(struct sockaddr));
		from.sin_family = AF_INET;
		from.sin_addr.s_addr = inet_addr(source);
		if (from.sin_addr.s_addr == -1)
			{
			    printf("%% unknown host %s\n", source);
			    exit (1);
			}    
		outpacket->ip.ip_src = from.sin_addr;
#ifndef IP_HDRINCL
		if (bind(sndsock, (struct sockaddr *)&from, sizeof(from)) < 0)
			{
			    printf("%% error bind\n");
			    exit (1);
			}
#endif
	}

	fprintf(stderr, "Tracing route to %s (%s)", hostname,
		inet_ntoa(to->sin_addr));
	if (source)
		fprintf(stderr, " from %s", source);
	fprintf(stderr, " over a maximum of %d hops, %d byte packets\n", max_ttl, datalen);

	for (ttl = 1; ttl <= max_ttl; ++ttl) {
		u_long lastaddr = 0;
		int got_there = 0;
		int unreachable = 0;

		printf("%2d ", ttl);
		for (probe = 0; probe < nprobes; ++probe) {
			int cc, reset_timer;
			struct timeval t1, t2;
			struct timezone tz;
			struct ip *ip;

			(void) gettimeofday(&t1, &tz);
			send_probe(++seq, ttl);
			reset_timer = 1;
			while ((cc = wait_for_reply(s, &from, reset_timer)) != 0) {
				(void) gettimeofday(&t2, &tz);
				if ((i = packet_ok(packet, cc, &from, seq))) {
					reset_timer = 1;
					if (from.sin_addr.s_addr != lastaddr) {
						print(packet, cc, &from);
						lastaddr = from.sin_addr.s_addr;
					}
					printf("  %g ms", deltaT(&t1, &t2));
					switch(i - 1) {
					case ICMP_UNREACH_PORT:
						ip = (struct ip *)packet;
						if (ip->ip_ttl <= 1)
							printf(" !");
						++got_there;
						break;
					case ICMP_UNREACH_NET:
						++unreachable;
						printf(" !N");
						break;
					case ICMP_UNREACH_HOST:
						++unreachable;
						printf(" !H");
						break;
					case ICMP_UNREACH_PROTOCOL:
						++got_there;
						printf(" !P");
						break;
					case ICMP_UNREACH_NEEDFRAG:
						++unreachable;
						printf(" !F");
						break;
					case ICMP_UNREACH_SRCFAIL:
						++unreachable;
						printf(" !S");
						break;
					}
					break;
				} else
					reset_timer = 0;
			}
			if (cc == 0)
				printf(" *");
			(void) fflush(stdout);
		}
		putchar('\n');
		if (got_there || unreachable >= nprobes-1)
			exit (0);
	}

	exit (0);
}
