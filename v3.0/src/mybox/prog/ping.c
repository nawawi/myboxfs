#include "libmybox.h"
#include <linux/filter.h>

#define	DEFDATALEN	(64 - 8)
#define	MAXWAIT		10
#define MININTERVAL	10
#define MINUSERINTERVAL	200
#define SCHINT(a)	(((a) <= MININTERVAL) ? MININTERVAL : (a))
#define	A(bit)		rcvd_tbl[(bit)>>3]
#define	B(bit)		(1 << ((bit) & 0x07))
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))
#define	F_INTERVAL	0x002
#define	F_NUMERIC	0x004
#define	F_QUIET		0x010
#define	F_SO_DONTROUTE	0x080
#define	F_VERBOSE	0x100
#define	F_FLOWINFO	0x200
#define	F_SOURCEROUTE	0x400
#define	F_TCLASS	0x400
#define	F_LATENCY	0x1000
#define	F_STRICTSOURCE	0x8000
#define F_NOLOOP	0x10000
#define F_TTL		0x20000
#define	MAX_DUP_CHK	0x10000

static int options;
static int mx_dup_ck = MAX_DUP_CHK;
static char rcvd_tbl[MAX_DUP_CHK / 8];
u_char outpack[];
static int maxpacket;
static int datalen = DEFDATALEN;;
static char *hostname;
static int uid;
static int ident;
static int sndbuf;
static int ttl;
static long npackets;
static long nreceived;
static long nrepeats;
static long ntransmitted;
static long nchecksum;
static long nerrors;
static int interval = 1000;
static int preload;
static int deadline = 0;
static int lingertime = MAXWAIT*1000;
struct timeval start_time, cur_time;
volatile int exiting;
volatile int status_snapshot;
static int confirm = 0;
static int confirm_flag = 0;
static int working_recverr;
static int timing;
static long tmin=LONG_MAX;
static long tmax;
static long long tsum;
static long long tsum2;
static int pipesize = -1;
static int screen_width = INT_MAX;
static int rtt;
 __u16 acked;
static int rtt_addend;

static int __schedule_exit(int next);
int send_probe(void);
int receive_error_msg(void);
int parse_reply(struct msghdr *msg, int len, void *addr, struct timeval *);
void install_filter(void);
int pinger(void);
void sock_setbufs(int icmp_sock, int alloc);
int setup(int icmp_sock);
int main_loop(int icmp_sock, __u8 *buf, int buflen);
int finish(void);
void status(void);
int common_options(int ch);
int gather_statistics(__u8 *ptr, int cc, __u16 seq, int hops,int csfailed, struct timeval *tv, char *from);


#define COMMON_OPTIONS case 'i': case 'c': case 'w': case 'q': case 'W':

#define COMMON_OPTSTR "h?VQ:I:M:aUc:dfi:w:l:S:np:qrs:vLt:AW:B"

static inline void tvsub(struct timeval *out, struct timeval *in) {
	if ((out->tv_usec -= in->tv_usec) < 0) {
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

static inline void set_signal(int signo, void (*handler)(int)) {
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = (void (*)(int))handler;
#ifdef SA_INTERRUPT
	sa.sa_flags = SA_INTERRUPT;
#endif
	sigaction(signo, &sa, NULL);
}

static inline int schedule_exit(int next) {
	if (npackets && ntransmitted >= npackets && !deadline) next = __schedule_exit(next);
	return next;
}

static inline int in_flight(void) {
	__u16 diff = (__u16)ntransmitted - acked;
	return (diff<=0x7FFF) ? diff : ntransmitted-nreceived-nerrors;
}

static inline void acknowledge(__u16 seq) { 
	__u16 diff = (__u16)ntransmitted - seq;
	if (diff <= 0x7FFF) {
		if ((int)diff+1 > pipesize)
			pipesize = (int)diff+1;
		if ((__s16)(seq - acked) > 0 ||
		    (__u16)ntransmitted - acked > 0x7FFF)
			acked = seq;
	}
}

static inline void advance_ntransmitted(void) {
	ntransmitted++;
	if ((__u16)ntransmitted - acked > 0x7FFF) acked = (__u16)ntransmitted + 1;
}



int common_options(int ch) {
	switch(ch) {
		case 'c':
			npackets = atoi(optarg);
			if(npackets <= 0) {
				fprintf(stderr, "%% bad number of packets to transmit.\n");
				return(1);
			}
			break;
		case 'i': {
			if(strchr(optarg, '.')) {
				float t;
				if(sscanf(optarg, "%f", &t) != 1) {
					fprintf(stderr, "%% bad timing interval.\n");
					return(1);
				}
				interval = (int)(t*1000);
			} else if (sscanf(optarg, "%d", &interval) == 1) {
				interval *= 1000;
			} else {
				fprintf(stderr, "%% bad timing interval.\n");
				return(1);
			}

			if (interval < 0) {
				fprintf(stderr, "%% bad timing interval.\n");
				return(1);
			}
			options |= F_INTERVAL;
			break;
		}
		case 'w':
			deadline = atoi(optarg);
			if (deadline < 0) {
				fprintf(stderr, "%% bad wait time.\n");
				return(1);
			}
			break;
		case 'q':
			options |= F_QUIET;
			break;
		case 'W':
			lingertime = atoi(optarg);
			if (lingertime < 0 || lingertime > INT_MAX/1000000) {
				fprintf(stderr, "%% bad linger time.\n");
				return(1);
			}
			lingertime *= 1000;
			break;
		default:
			abort();
	}
	return(0);
}


static void sigexit(int signo) {
	exiting = 1;
}

static void sigstatus(int signo) {
	status_snapshot = 1;
}


static int __schedule_exit(int next) {
	static unsigned long waittime;
	struct itimerval it;

	if(waittime) return next;
	if(nreceived) {
		waittime = 2 * tmax;
		if(waittime < 1000*interval) waittime = 1000*interval;
	} else {
		waittime = lingertime*1000;
	}

	if (next < 0 || next < waittime/1000) next = waittime/1000;

	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 0;
	it.it_value.tv_sec = waittime/1000000;
	it.it_value.tv_usec = waittime%1000000;
	setitimer(ITIMER_REAL, &it, NULL);
	return next;
}

int pinger(void) {
	static int oom_count;
	static int tokens;
	int i;
	ssize_t wr_ret;

	if (exiting || (npackets && ntransmitted >= npackets && !deadline)) return 1000;

	if (cur_time.tv_sec == 0) {
		gettimeofday(&cur_time, NULL);
		tokens = interval*(preload-1);
	} else {
		long ntokens;
		struct timeval tv;

		gettimeofday(&tv, NULL);
		ntokens = (tv.tv_sec - cur_time.tv_sec)*1000 +
			(tv.tv_usec-cur_time.tv_usec)/1000;
		ntokens += tokens;
		if (ntokens > interval*preload)
			ntokens = interval*preload;
		if (ntokens < interval)
			return interval - ntokens;

		cur_time = tv;
		tokens = ntokens - interval;
	}

resend:
	i = send_probe();

	if (i == 0) {
		oom_count = 0;
		advance_ntransmitted();
		return interval - tokens;
	}

	/* And handle various errors... */
	if (i > 0) {
		/* Apparently, it is some fatal bug. */
		abort();
	} else if (errno == ENOBUFS || errno == ENOMEM) {
		int nores_interval;

		/* Device queue overflow or OOM. Packet is not sent. */
		tokens = 0;
		/* Slowdown. This works only in adaptive mode (option -A) */
		rtt_addend += (rtt < 8*50000 ? rtt/8 : 50000);
		nores_interval = SCHINT(interval/2);
		if (nores_interval > 500)
			nores_interval = 500;
		oom_count++;
		if (oom_count*nores_interval < lingertime)
			return nores_interval;
		i = 0;
		/* Fall to hard error. It is to avoid complete deadlock
		 * on stuck output device even when dealine was not requested.
		 * Expected timings are screwed up in any case, but we will
		 * exit some day. :-) */
	} else if (errno == EAGAIN) {
		/* Socket buffer is full. */
		tokens += interval;
		return MININTERVAL;
	} else {
		if ((i=receive_error_msg()) > 0) {
			/* An ICMP error arrived. */
			tokens += interval;
			return MININTERVAL;
		}
		/* Compatibility with old linuces. */
		if (i == 0 && confirm_flag && errno == EINVAL) {
			confirm_flag = 0;
			errno = 0;
		}
		if (!errno)
			goto resend;
	}

	/* Hard local error. Pretend we sent packet. */
	advance_ntransmitted();

	if (i == 0 && !(options & F_QUIET)) {
		perror("ping: sendmsg");
	}
	tokens = 0;
	return SCHINT(interval);
}

/* Set socket buffers, "alloc" is an estimate of memory taken by single packet. */

void sock_setbufs(int icmp_sock, int alloc)
{
	int rcvbuf, hold;
	socklen_t tmplen = sizeof(hold);

	if (!sndbuf)
		sndbuf = alloc;
	setsockopt(icmp_sock, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, sizeof(sndbuf));

	rcvbuf = hold = alloc * preload;
	if (hold < 65536)
		hold = 65536;
	setsockopt(icmp_sock, SOL_SOCKET, SO_RCVBUF, (char *)&hold, (socklen_t)sizeof(hold));
	if (getsockopt(icmp_sock, SOL_SOCKET, SO_RCVBUF, (char *)&hold, (socklen_t*)&tmplen) == 0) {
		if (hold < rcvbuf)
			fprintf(stderr, "WARNING: probably, rcvbuf is not enough to hold preload.\n");
	}
}

/* Protocol independent setup and parameter checks. */

int setup(int icmp_sock)
{
	int hold;
	struct timeval tv;

	if (interval >= INT_MAX/preload) {
		fprintf(stderr, "ping: illegal preload and/or interval\n");
		return(1);
	}

	hold = 1;
	if (options & F_SO_DONTROUTE)
		setsockopt(icmp_sock, SOL_SOCKET, SO_DONTROUTE, (char *)&hold, sizeof(hold));

#ifdef SO_TIMESTAMP
	if (!(options&F_LATENCY)) {
		int on = 1;
		if (setsockopt(icmp_sock, SOL_SOCKET, SO_TIMESTAMP, &on, sizeof(on)))
			fprintf(stderr, "Warning: no SO_TIMESTAMP support, falling back to SIOCGSTAMP\n");
	}
#endif

	/* Set some SNDTIMEO to prevent blocking forever
	 * on sends, when device is too slow or stalls. Just put limit
	 * of one second, or "interval", if it is less.
	 */
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (interval < 1000) {
		tv.tv_sec = 0;
		tv.tv_usec = 1000 * SCHINT(interval);
	}
	setsockopt(icmp_sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));

	/* Set RCVTIMEO to "interval". Note, it is just an optimization
	 * allowing to avoid redundant poll(). */
	tv.tv_sec = SCHINT(interval)/1000;
	tv.tv_usec = 1000*(SCHINT(interval)%1000);
	int i;
	unsigned char *p = outpack+8;
	for (i = 0; i < datalen; ++i) *p++ = i;


	ident = getpid() & 0xFFFF;

	set_signal(SIGINT, sigexit);
	set_signal(SIGALRM, sigexit);
	set_signal(SIGQUIT, sigstatus);

	gettimeofday(&start_time, NULL);

	if (deadline) {
		struct itimerval it;

		it.it_interval.tv_sec = 0;
		it.it_interval.tv_usec = 0;
		it.it_value.tv_sec = deadline;
		it.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &it, NULL);
	}

	if (isatty(STDOUT_FILENO)) {
		struct winsize w;

		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
			if (w.ws_col > 0)
				screen_width = w.ws_col;
		}
	}
	return(0);
}

int main_loop(int icmp_sock, __u8 *packet, int packlen)
{
	char addrbuf[128];
	char ans_data[4096];
	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *c;
	int cc;
	int next;
	int polling;
	exiting=0;
	iov.iov_base = (char *)packet;

	for (;;) {
		/* Check exit conditions. */
		if (exiting)
			break;
		if (npackets && nreceived + nerrors >= npackets)
			break;
		if (deadline && nerrors)
			break;
		/* Check for and do special actions. */
		if (status_snapshot)
			status();

		/* Send probes scheduled to this time. */
		do {
			next = pinger();
			next = schedule_exit(next);
		} while (next <= 0);

		/* "next" is time to send next probe, if positive.
		 * If next<=0 send now or as soon as possible. */

		/* Technical part. Looks wicked. Could be dropped,
		 * if everyone used the newest kernel. :-) 
		 * Its purpose is:
		 * 1. Provide intervals less than resolution of scheduler.
		 *    Solution: spinning.
		 * 2. Avoid use of poll(), when recvmsg() can provide
		 *    timed waiting (SO_RCVTIMEO). */
		polling = 0;
		if (next<SCHINT(interval)) {
			int recv_expected = in_flight();

			/* If we are here, recvmsg() is unable to wait for
			 * required timeout. */ 
			if (1000*next <= 1000000/(int)HZ) {
				/* Very short timeout... So, if we wait for
				 * something, we sleep for MININTERVAL.
				 * Otherwise, spin! */
				if (recv_expected) {
					next = MININTERVAL;
				} else {
					next = 0;
					/* When spinning, no reasons to poll.
					 * Use nonblocking recvmsg() instead. */
					polling = MSG_DONTWAIT;
					/* But yield yet. */
					sched_yield();
				}
			}

			if (!polling && interval) {
				struct pollfd pset;
				pset.fd = icmp_sock;
				pset.events = POLLIN|POLLERR;
				pset.revents = 0;
				if (poll(&pset, 1, next) < 1 ||
				    !(pset.revents&(POLLIN|POLLERR)))
					continue;
				polling = MSG_DONTWAIT;
			}
		}

		for (;;) {
			struct timeval *recv_timep = NULL;
			struct timeval recv_time;
			int not_ours = 0; /* Raw socket can receive messages
					   * destined to other running pings. */

			iov.iov_len = packlen;
			memset(&msg,'\0',sizeof(msg));
			msg.msg_name = addrbuf;
			msg.msg_namelen = sizeof(addrbuf);
			msg.msg_iov = &iov;
			msg.msg_iovlen = 1;
			msg.msg_control = ans_data;
			msg.msg_controllen = sizeof(ans_data);

			cc = recvmsg(icmp_sock, &msg, polling);
			polling = MSG_DONTWAIT;

			if (cc < 0) {
				if (errno == EAGAIN || errno == EINTR)
					break;
				if (!receive_error_msg()) {
					if (errno) {
						perror("ping: recvmsg");
						break;
					}
					not_ours = 1;
				}
			} else {

#ifdef SO_TIMESTAMP
				for (c = CMSG_FIRSTHDR(&msg); c; c = CMSG_NXTHDR(&msg, c)) {
					if (c->cmsg_level != SOL_SOCKET ||
					    c->cmsg_type != SO_TIMESTAMP)
						continue;
					if (c->cmsg_len < CMSG_LEN(sizeof(struct timeval)))
						continue;
					recv_timep = (struct timeval*)CMSG_DATA(c);
				}
#endif

				if ((options&F_LATENCY) || recv_timep == NULL) {
					if ((options&F_LATENCY) ||
					    ioctl(icmp_sock, SIOCGSTAMP, &recv_time))
						gettimeofday(&recv_time, NULL);
					recv_timep = &recv_time;
				}

				not_ours = parse_reply(&msg, cc, addrbuf, recv_timep);
			}

			/* See? ... someone runs another ping on this host. */ 
			if (not_ours)
				install_filter();

			/* If nothing is in flight, "break" returns us to pinger. */
			if (in_flight() == 0)
				break;

			/* Otherwise, try to recvmsg() again. recvmsg()
			 * is nonblocking after the first iteration, so that
			 * if nothing is queued, it will receive EAGAIN
			 * and return to pinger. */
		}
	}
	return finish();
}

int gather_statistics(__u8 *ptr, int cc, __u16 seq, int hops,
		      int csfailed, struct timeval *tv, char *from)
{
	int dupflag = 0;
	long triptime = 0;
	ssize_t wr_ret;
	
	++nreceived;
	if (!csfailed)
		acknowledge(seq);

	if (timing && cc >= 8+sizeof(struct timeval)) {
		struct timeval tmp_tv;
		memcpy(&tmp_tv, ptr, sizeof(tmp_tv));

restamp:
		tvsub(tv, &tmp_tv);
		triptime = tv->tv_sec * 1000000 + tv->tv_usec;
		if (triptime < 0) {
			if (options & F_VERBOSE)
				fprintf(stderr, "Warning: time of day goes back (%ldus), taking countermeasures.\n", triptime);
			triptime = 0;
			if (!(options & F_LATENCY)) {
				gettimeofday(tv, NULL);
				options |= F_LATENCY;
				goto restamp;
			}
		}
		if (!csfailed) {
			tsum += triptime;
			tsum2 += (long long)triptime * (long long)triptime;
			if (triptime < tmin)
				tmin = triptime;
			if (triptime > tmax)
				tmax = triptime;
			if (!rtt)
				rtt = triptime*8;
			else
				rtt += triptime-rtt/8;
		}
	}

	if (csfailed) {
		++nchecksum;
		--nreceived;
	} else if (TST(seq % mx_dup_ck)) {
		++nrepeats;
		--nreceived;
		dupflag = 1;
	} else {
		SET(seq % mx_dup_ck);
		dupflag = 0;
	}
	confirm = confirm_flag;

	if (options & F_QUIET)
		return 1;


		int i;
		__u8 *cp, *dp;
		printf("%d bytes from %s: icmp_seq=%u", cc, from, seq);

		if (hops >= 0)
			printf(" ttl=%d", hops);

		if (cc < datalen+8) {
			printf(" (truncated)\n");
			return 1;
		}
		if (timing) {
			if (triptime >= 100000)
				printf(" time=%ld ms", triptime/1000);
			else if (triptime >= 10000)
				printf(" time=%ld.%01ld ms", triptime/1000,
				       (triptime%1000)/100);
			else if (triptime >= 1000)
				printf(" time=%ld.%02ld ms", triptime/1000,
				       (triptime%1000)/10);
			else
				printf(" time=%ld.%03ld ms", triptime/1000,
				       triptime%1000);
		}
		if (dupflag)
			printf(" (DUP!)");
		if (csfailed)
			printf(" (BAD CHECKSUM!)");

		/* check the data */
		cp = ((u_char*)ptr) + sizeof(struct timeval);
		dp = &outpack[8 + sizeof(struct timeval)];
		for (i = sizeof(struct timeval); i < datalen; ++i, ++cp, ++dp) {
			if (*cp != *dp) {
				printf("\nwrong data byte #%d should be 0x%x but was 0x%x",
				       i, *dp, *cp);
				cp = (u_char*)ptr + sizeof(struct timeval);
				for (i = sizeof(struct timeval); i < datalen; ++i, ++cp) {
					if ((i % 32) == sizeof(struct timeval))
						printf("\n#%d\t", i);
					printf("%x ", *cp);
				}
				break;
			}
		}

	return 0;
}

static long llsqrt(long long a)
{
	long long prev = ~((long long)1 << 63);
	long long x = a;

	if (x > 0) {
		while (x < prev) {
			prev = x;
			x = (x+(a/x))/2;
		}
	}

	return (long)x;
}

/*
 * finish --
 *	Print out statistics, and give up.
 */
int finish(void)
{
	struct timeval tv = cur_time;

	tvsub(&tv, &start_time);

	putchar('\n');
	fflush(stdout);
	printf("--- %s ping statistics ---\n", hostname);
	printf("%ld packets transmitted, ", ntransmitted);
	printf("%ld received", nreceived);
	if (nrepeats)
		printf(", +%ld duplicates", nrepeats);
	if (nchecksum)
		printf(", +%ld corrupted", nchecksum);
	if (nerrors)
		printf(", +%ld errors", nerrors);
	if (ntransmitted) {
		printf(", %d%% packet loss",
		       (int) ((((long long)(ntransmitted - nreceived)) * 100) /
			      ntransmitted));
		printf(", time %ldms", 1000*tv.tv_sec+tv.tv_usec/1000);
	}
	putchar('\n');

	if (nreceived && timing) {
		long tmdev;

		tsum /= nreceived + nrepeats;
		tsum2 /= nreceived + nrepeats;
		tmdev = llsqrt(tsum2 - tsum * tsum);

		printf("rtt min/avg/max/mdev = %ld.%03ld/%lu.%03ld/%ld.%03ld/%ld.%03ld ms",
		       (long)tmin/1000, (long)tmin%1000,
		       (unsigned long)(tsum/1000), (long)(tsum%1000),
		       (long)tmax/1000, (long)tmax%1000,
		       (long)tmdev/1000, (long)tmdev%1000
		       );
	}
	if (pipesize > 1)
		printf(", pipe %d", pipesize);
	if (ntransmitted > 1 && nreceived) {
		int ipg = (1000000*(long long)tv.tv_sec+tv.tv_usec)/(ntransmitted-1);
		printf(", ipg/ewma %d.%03d/%d.%03d ms",
		       ipg/1000, ipg%1000, rtt/8000, (rtt/8)%1000);
	}
	putchar('\n');
	return(!nreceived || (deadline && nreceived < npackets));
}


void status(void)
{
	int loss = 0;
	long tavg = 0;

	status_snapshot = 0;

	if (ntransmitted)
		loss = (((long long)(ntransmitted - nreceived)) * 100) / ntransmitted;

	fprintf(stderr, "\r%ld/%ld packets, %d%% loss", ntransmitted, nreceived, loss);

	if (nreceived && timing) {
		tavg = tsum / (nreceived + nrepeats);

		fprintf(stderr, ", min/avg/ewma/max = %ld.%03ld/%lu.%03ld/%d.%03d/%ld.%03ld ms",
		       (long)tmin/1000, (long)tmin%1000,
		       tavg/1000, tavg%1000,
		       rtt/8000, (rtt/8)%1000,
		       (long)tmax/1000, (long)tmax%1000
		       );
	}
	fprintf(stderr, "\n");
}



#define	MAXIPLEN	60
#define	MAXICMPLEN	76
#define	NROUTES		9		/* number of record route slots */
#define TOS_MAX		255		/* 8-bit TOS field */


static int ts_type;
static int nroute = 0;
static __u32 route[10];



struct sockaddr_in whereto;	/* who to ping */
int optlen = 0;
int settos = 0;			/* Set TOS, Precendence or other QOS options */
int icmp_sock;			/* socket file descriptor */
u_char outpack[0x10000];
static int maxpacket = sizeof(outpack);

static char *pr_addr(__u32);
static void pr_options(unsigned char * cp, int hlen);
static void pr_iph(struct iphdr *ip);
static u_short in_cksum(const u_short *addr, int len, u_short salt);
static void pr_icmph(__u8 type, __u8 code, __u32 info, struct icmphdr *icp);

static struct {
	struct cmsghdr cm;
	struct in_pktinfo ipi;
} cmsg = { {sizeof(struct cmsghdr) + sizeof(struct in_pktinfo), SOL_IP, IP_PKTINFO},
	   {0, }};
int cmsg_len;

struct sockaddr_in source;
char *device=NULL;

int ping_main(int argc, char **argv)
{
	struct hostent *hp;
	int ch, hold, packlen;
	int socket_errno;
	u_char *packet;
	char *target, hnamebuf[MAXHOSTNAMELEN];
	char rspace[3 + 4 * NROUTES + 1];	/* record route space */

	icmp_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	socket_errno = errno;

	uid = getuid();
	if(setuid(uid) != 0)
	{
		perror("ping: setuid()");
		return(1);
	}

	source.sin_family = AF_INET;

	preload = 1;
	while ((ch = getopt(argc, argv, COMMON_OPTSTR "RT:")) != EOF) {
		switch(ch) {
		case 'I':
		{
			char dummy;
			int i1, i2, i3, i4;

			if (sscanf(optarg, "%u.%u.%u.%u%c",
				   &i1, &i2, &i3, &i4, &dummy) == 4) {
				__u8 *ptr;
				ptr = (__u8*)&source.sin_addr;
				ptr[0] = i1;
				ptr[1] = i2;
				ptr[2] = i3;
				ptr[3] = i4;
				options |= F_STRICTSOURCE;
			} else {
				device = strdup(optarg);
			}
			break;
		}
		COMMON_OPTIONS
			common_options(ch);
			break;
		default:
			return(1);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0) 
		return(1);
	if (argc > 1) {
		if (argc > 10) return(1);
		options |= F_SOURCEROUTE;
	}
	while (argc > 0) {
		target = *argv;

		bzero((char *)&whereto, sizeof(whereto));
		whereto.sin_family = AF_INET;
		if (inet_aton(target, &whereto.sin_addr) == 1) {
			hostname = target;
			if (argc == 1)
				options |= F_NUMERIC;
		} else {
			hp = gethostbyname(target);
			if (!hp) {
				fprintf(stderr, "ping: unknown host %s\n", target);
				return(1);
			}
			memcpy(&whereto.sin_addr, hp->h_addr, 4);
			strncpy(hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
			hnamebuf[sizeof(hnamebuf) - 1] = 0;
			hostname = hnamebuf;
		}
		if (argc > 1)
			route[nroute++] = whereto.sin_addr.s_addr;
		argc--;
		argv++;
	}

	if (source.sin_addr.s_addr == 0) {
		socklen_t alen;
		struct sockaddr_in dst = whereto;
		int probe_fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (probe_fd < 0) {
			perror("socket");
			return(1);
		}
		if (device) {
			struct ifreq ifr;
			memset(&ifr, 0, sizeof(ifr));
			strncpy(ifr.ifr_name, device, IFNAMSIZ-1);
			if (setsockopt(probe_fd, SOL_SOCKET, SO_BINDTODEVICE, device, strlen(device)+1) == -1) {
				if (IN_MULTICAST(ntohl(dst.sin_addr.s_addr))) {
					struct ip_mreqn imr;
					if (ioctl(probe_fd, SIOCGIFINDEX, &ifr) < 0) {
						fprintf(stderr, "ping: unknown iface %s\n", device);
						return(1);
					}
					memset(&imr, 0, sizeof(imr));
					imr.imr_ifindex = ifr.ifr_ifindex;
					if (setsockopt(probe_fd, SOL_IP, IP_MULTICAST_IF, &imr, sizeof(imr)) == -1) {
						perror("ping: IP_MULTICAST_IF");
						return(1);
					}
				} else if (icmp_sock >= 0) {
					/* We possible tried to SO_BINDTODEVICE() a subinterface like 'eth0:1' */
					perror("Warning: cannot bind to specified iface, falling back");
				}
			}
		}

		if (settos &&
		    setsockopt(probe_fd, IPPROTO_IP, IP_TOS, (char *)&settos, sizeof(int)) < 0)
			perror("Warning: error setting QOS sockopts");

		dst.sin_port = htons(1025);
		if (nroute)
			dst.sin_addr.s_addr = route[0];
		if (connect(probe_fd, (struct sockaddr*)&dst, sizeof(dst)) == -1) {
			if (errno == EACCES) {
				if (connect(probe_fd, (struct sockaddr*)&dst, sizeof(dst)) == -1) {
					perror("connect");
					return(1);
				}
			} else {
				perror("connect");
				return(1);
			}
		}
		alen = sizeof(source);
		if (getsockname(probe_fd, (struct sockaddr*)&source, (socklen_t*)&alen) == -1) {
			perror("getsockname");
			return(1);
		}
		source.sin_port = 0;
		close(probe_fd);
	} while (0);

	if (whereto.sin_addr.s_addr == 0)
		whereto.sin_addr.s_addr = source.sin_addr.s_addr;

	if (icmp_sock < 0) {
		errno = socket_errno;
		perror("ping: icmp open socket");
		return(1);
	}

	if (device) {
		struct ifreq ifr;

		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, device, IFNAMSIZ-1);
		if (ioctl(icmp_sock, SIOCGIFINDEX, &ifr) < 0) {
			fprintf(stderr, "ping: unknown iface %s\n", device);
			return(1);
		}
		cmsg.ipi.ipi_ifindex = ifr.ifr_ifindex;
		cmsg_len = sizeof(cmsg);
	}

	if ((options&F_STRICTSOURCE) &&
	    bind(icmp_sock, (struct sockaddr*)&source, sizeof(source)) == -1) {
		perror("bind");
		return(1);
	}

	if (1) {
		struct icmp_filter filt;
		filt.data = ~((1<<ICMP_SOURCE_QUENCH)|
			      (1<<ICMP_DEST_UNREACH)|
			      (1<<ICMP_TIME_EXCEEDED)|
			      (1<<ICMP_PARAMETERPROB)|
			      (1<<ICMP_REDIRECT)|
			      (1<<ICMP_ECHOREPLY));
		if (setsockopt(icmp_sock, SOL_RAW, ICMP_FILTER, (char*)&filt, sizeof(filt)) == -1)
			perror("WARNING: setsockopt(ICMP_FILTER)");
	}

	hold = 1;
	if (setsockopt(icmp_sock, SOL_IP, IP_RECVERR, (char *)&hold, sizeof(hold)))
		fprintf(stderr, "WARNING: your kernel is veeery old. No problems.\n");


	if (options & F_SOURCEROUTE) {
	        int i;
	        bzero(rspace, sizeof(rspace));
		rspace[0] = IPOPT_NOOP;
		rspace[1+IPOPT_OPTVAL] = (options & F_SO_DONTROUTE) ? IPOPT_SSRR
			: IPOPT_LSRR;
		rspace[1+IPOPT_OLEN] = 3 + nroute*4;
		rspace[1+IPOPT_OFFSET] = IPOPT_MINOFF;
		for (i=0; i<nroute; i++)
			*(__u32*)&rspace[4+i*4] = route[i];
		
		if (setsockopt(icmp_sock, IPPROTO_IP, IP_OPTIONS, rspace, 4 + nroute*4) < 0) {
			perror("ping: record route");
			return(1);
		}
		optlen = 40;
	}

	/* Estimate memory eaten by single packet. It is rough estimate.
	 * Actually, for small datalen's it depends on kernel side a lot. */
	hold = datalen + 8;
	hold += ((hold+511)/512)*(optlen + 20 + 16 + 64 + 160);
	sock_setbufs(icmp_sock, hold);

	if (options & F_NOLOOP) {
		int loop = 0;
		if (setsockopt(icmp_sock, IPPROTO_IP, IP_MULTICAST_LOOP,
							&loop, 1) == -1) {
			perror ("ping: can't disable multicast loopback");
			return(1);
		}
	}
	if (options & F_TTL) {
		int ittl = ttl;
		if (setsockopt(icmp_sock, IPPROTO_IP, IP_MULTICAST_TTL,
							&ttl, 1) == -1) {
			perror ("ping: can't set multicast time-to-live");
			return(1);
		}
		if (setsockopt(icmp_sock, IPPROTO_IP, IP_TTL,
							&ittl, sizeof(ittl)) == -1) {
			perror ("ping: can't set unicast time-to-live");
			return(1);
		}
	}

	if (datalen > 0xFFFF - 8 - optlen - 20) {
		if (uid || datalen > sizeof(outpack)-8) {
			fprintf(stderr, "Error: packet size %d is too large. Maximum is %d\n", datalen, 0xFFFF-8-20-optlen);
			return(1);
		}
		/* Allow small oversize to root yet. It will cause EMSGSIZE. */
		fprintf(stderr, "WARNING: packet size %d is too large. Maximum is %d\n", datalen, 0xFFFF-8-20-optlen);
	}

	if (datalen >= sizeof(struct timeval))	/* can we time transfer */
		timing = 1;
	packlen = datalen + MAXIPLEN + MAXICMPLEN;
	if (!(packet = (u_char *)malloc((u_int)packlen))) {
		fprintf(stderr, "ping: out of memory.\n");
		return(1);
	}

	printf("PING %s (%s) ", hostname, inet_ntoa(whereto.sin_addr));
	if (device || (options&F_STRICTSOURCE))
		printf("from %s %s: ", inet_ntoa(source.sin_addr), device ?: "");
	printf("%d(%d) bytes of data.\n", datalen, datalen+8+optlen+20);

	setup(icmp_sock);

	return main_loop(icmp_sock, packet, packlen);
}


int receive_error_msg()
{
	int res;
	char cbuf[512];
	struct iovec  iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct sock_extended_err *e;
	struct icmphdr icmph;
	struct sockaddr_in target;
	int net_errors = 0;
	int local_errors = 0;
	int saved_errno = errno;
	ssize_t wr_ret;

	iov.iov_base = &icmph;
	iov.iov_len = sizeof(icmph);
	msg.msg_name = (void*)&target;
	msg.msg_namelen = sizeof(target);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;
	msg.msg_control = cbuf;
	msg.msg_controllen = sizeof(cbuf);

	res = recvmsg(icmp_sock, &msg, MSG_ERRQUEUE|MSG_DONTWAIT);
	if (res < 0)
		goto out;

	e = NULL;
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_IP) {
			if (cmsg->cmsg_type == IP_RECVERR)
				e = (struct sock_extended_err *)CMSG_DATA(cmsg);
		}
	}
	if (e == NULL)
		abort();

	if (e->ee_origin == SO_EE_ORIGIN_LOCAL) {
		local_errors++;
		if (options & F_QUIET)
			goto out;
		if (e->ee_errno != EMSGSIZE)
			fprintf(stderr, "ping: local error: %s\n", strerror(e->ee_errno));
		else
			fprintf(stderr, "ping: local error: Message too long, mtu=%u\n", e->ee_info);
		nerrors++;
	} else if (e->ee_origin == SO_EE_ORIGIN_ICMP) {
		struct sockaddr_in *sin = (struct sockaddr_in*)(e+1);

		if (res < sizeof(icmph) ||
		    target.sin_addr.s_addr != whereto.sin_addr.s_addr ||
		    icmph.type != ICMP_ECHO ||
		    icmph.un.echo.id != ident) {
			/* Not our error, not an error at all. Clear. */
			saved_errno = 0;
			goto out;
		}

		acknowledge(ntohs(icmph.un.echo.sequence));

		if (!working_recverr) {
			struct icmp_filter filt;
			working_recverr = 1;
			/* OK, it works. Add stronger filter. */
			filt.data = ~((1<<ICMP_SOURCE_QUENCH)|
				      (1<<ICMP_REDIRECT)|
				      (1<<ICMP_ECHOREPLY));
			if (setsockopt(icmp_sock, SOL_RAW, ICMP_FILTER, (char*)&filt, sizeof(filt)) == -1)
				perror("\rWARNING: setsockopt(ICMP_FILTER)");
		}

		net_errors++;
		nerrors++;
		if (options & F_QUIET) goto out;
		printf("From %s icmp_seq=%u ", pr_addr(sin->sin_addr.s_addr), ntohs(icmph.un.echo.sequence));
		pr_icmph(e->ee_type, e->ee_code, e->ee_info, NULL);
		fflush(stdout);
	}

out:
	errno = saved_errno;
	return net_errors ? : -local_errors;
}

/*
 * pinger --
 * 	Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a UNIX "timeval" struct in VAX
 * byte-order, to compute the round-trip time.
 */
int send_probe()
{
	struct icmphdr *icp;
	int cc;
	int i;

	icp = (struct icmphdr *)outpack;
	icp->type = ICMP_ECHO;
	icp->code = 0;
	icp->checksum = 0;
	icp->un.echo.sequence = htons(ntransmitted+1);
	icp->un.echo.id = ident;			/* ID */

	CLR((ntransmitted+1) % mx_dup_ck);

	if (timing) {
		if (options&F_LATENCY) {
			static volatile int fake_fucked_egcs = sizeof(struct timeval);
			struct timeval tmp_tv;
			gettimeofday(&tmp_tv, NULL);
			/* egcs is crap or glibc is crap, but memcpy 
			   does not copy anything, if len is constant! */
			memcpy(icp+1, &tmp_tv, fake_fucked_egcs);
		} else {
			memset(icp+1, 0, sizeof(struct timeval));
		}
	}

	cc = datalen + 8;			/* skips ICMP portion */

	/* compute ICMP checksum here */
	icp->checksum = in_cksum((u_short *)icp, cc, 0);

	if (timing && !(options&F_LATENCY)) {
		static volatile int fake_fucked_egcs = sizeof(struct timeval);
	        struct timeval tmp_tv;
		gettimeofday(&tmp_tv, NULL);
		/* egcs is crap or glibc is crap, but memcpy 
		   does not copy anything, if len is constant! */
		memcpy(icp+1, &tmp_tv, fake_fucked_egcs);
		icp->checksum = in_cksum((u_short *)(icp+1), fake_fucked_egcs, ~icp->checksum);
	}

        do {
		static struct iovec iov = {outpack, 0};
		static struct msghdr m = { &whereto, sizeof(whereto),
						   &iov, 1, &cmsg, 0, 0 };
		m.msg_controllen = cmsg_len;
		iov.iov_len = cc;

		i = sendmsg(icmp_sock, &m, confirm);
		confirm = 0;
	} while (0);

	return (cc == i ? 0 : i);
}

/*
 * parse_reply --
 *	Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
int
parse_reply(struct msghdr *msg, int cc, void *addr, struct timeval *tv)
{
	struct sockaddr_in *from = addr;
	__u8 *buf = msg->msg_iov->iov_base;
	struct icmphdr *icp;
	struct iphdr *ip;
	int hlen;
	int csfailed;
	ssize_t wr_ret;

	/* Check the IP header */
	ip = (struct iphdr *)buf;
	hlen = ip->ihl*4;
	if (cc < hlen + 8 || ip->ihl < 5) {
		if (options & F_VERBOSE)
			fprintf(stderr, "ping: packet too short (%d bytes) from %s\n", cc,
				pr_addr(from->sin_addr.s_addr));
		return 1;
	}

	/* Now the ICMP part */
	cc -= hlen;
	icp = (struct icmphdr *)(buf + hlen);
	csfailed = in_cksum((u_short *)icp, cc, 0);

	if (icp->type == ICMP_ECHOREPLY) {
		if (icp->un.echo.id != ident)
			return 1;			/* 'Twas not our ECHO */
		if (gather_statistics((__u8*)(icp+1), cc,
				      ntohs(icp->un.echo.sequence),
				      ip->ttl, 0, tv, pr_addr(from->sin_addr.s_addr)))
			return 0;
	} else {
		/* We fall here when a redirect or source quench arrived.
		 * Also this branch processes icmp errors, when IP_RECVERR
		 * is broken. */
		   
	        switch (icp->type) {
		case ICMP_ECHO:
			/* MUST NOT */
			return 1;
		case ICMP_SOURCE_QUENCH:
		case ICMP_REDIRECT:
		case ICMP_DEST_UNREACH:
		case ICMP_TIME_EXCEEDED:
		case ICMP_PARAMETERPROB:
			{
				struct iphdr * iph = (struct  iphdr *)(&icp[1]);
				struct icmphdr *icp1 = (struct icmphdr*)((unsigned char *)iph + iph->ihl*4);
				int error_pkt;
				if (cc < 8+sizeof(struct iphdr)+8 ||
				    cc < 8+iph->ihl*4+8)
					return 1;
				if (icp1->type != ICMP_ECHO ||
				    iph->daddr != whereto.sin_addr.s_addr ||
				    icp1->un.echo.id != ident)
					return 1;
				error_pkt = (icp->type != ICMP_REDIRECT &&
					     icp->type != ICMP_SOURCE_QUENCH);
				if (error_pkt) {
					acknowledge(ntohs(icp1->un.echo.sequence));
					if (working_recverr) {
						return 0;
					} else {
						static int once;
						/* Sigh, IP_RECVERR for raw socket
						 * was broken until 2.4.9. So, we ignore
						 * the first error and warn on the second.
						 */
						if (once++ == 1)
							fprintf(stderr, "\rWARNING: kernel is not very fresh, upgrade is recommended.\n");
						if (once == 1)
							return 0;
					}
				}
				nerrors+=error_pkt;
				if (options&F_QUIET)
					return !error_pkt;
				printf("From %s: icmp_seq=%u ",
				       pr_addr(from->sin_addr.s_addr),
				       ntohs(icp1->un.echo.sequence));
				if (csfailed)
					printf("(BAD CHECKSUM)");
				pr_icmph(icp->type, icp->code, ntohl(icp->un.gateway), icp);
				return !error_pkt;
			}
	        default:
			/* MUST NOT */
			break;
		}
		if (!(options & F_VERBOSE) || uid)
			return 0;
		printf("From %s: ", pr_addr(from->sin_addr.s_addr));
		if (csfailed) {
			printf("(BAD CHECKSUM)\n");
			return 0;
		}
		pr_icmph(icp->type, icp->code, ntohl(icp->un.gateway), icp);
		return 0;
	}

	pr_options(buf + sizeof(struct iphdr), hlen);
	putchar('\n');
	fflush(stdout);

	return 0;
}

u_short
in_cksum(const u_short *addr, register int len, u_short csum)
{
	register int nleft = len;
	const u_short *w = addr;
	register u_short answer;
	register int sum = csum;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1)
		sum += htons(*(u_char *)w << 8);

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}

/*
 * pr_icmph --
 *	Print a descriptive string about an ICMP header.
 */
void pr_icmph(__u8 type, __u8 code, __u32 info, struct icmphdr *icp)
{
	switch(type) {
	case ICMP_ECHOREPLY:
		printf("Echo Reply\n");
		/* XXX ID + Seq + Data */
		break;
	case ICMP_DEST_UNREACH:
		switch(code) {
		case ICMP_NET_UNREACH:
			printf("Destination Net Unreachable\n");
			break;
		case ICMP_HOST_UNREACH:
			printf("Destination Host Unreachable\n");
			break;
		case ICMP_PROT_UNREACH:
			printf("Destination Protocol Unreachable\n");
			break;
		case ICMP_PORT_UNREACH:
			printf("Destination Port Unreachable\n");
			break;
		case ICMP_FRAG_NEEDED:
			printf("Frag needed and DF set (mtu = %u)\n", info);
			break;
		case ICMP_SR_FAILED:
			printf("Source Route Failed\n");
			break;
		case ICMP_NET_UNKNOWN:
			printf("Destination Net Unknown\n");
			break;
		case ICMP_HOST_UNKNOWN:
			printf("Destination Host Unknown\n");
			break;
		case ICMP_HOST_ISOLATED:
			printf("Source Host Isolated\n");
			break;
		case ICMP_NET_ANO:
			printf("Destination Net Prohibited\n");
			break;
		case ICMP_HOST_ANO:
			printf("Destination Host Prohibited\n");
			break;
		case ICMP_NET_UNR_TOS:
			printf("Destination Net Unreachable for Type of Service\n");
			break;
		case ICMP_HOST_UNR_TOS:
			printf("Destination Host Unreachable for Type of Service\n");
			break;
		case ICMP_PKT_FILTERED:
			printf("Packet filtered\n");
			break;
		case ICMP_PREC_VIOLATION:
			printf("Precedence Violation\n");
			break;
		case ICMP_PREC_CUTOFF:
			printf("Precedence Cutoff\n");
			break;
		default:
			printf("Dest Unreachable, Bad Code: %d\n", code);
			break;
		}
		if (icp && (options & F_VERBOSE))
			pr_iph((struct iphdr*)(icp + 1));
		break;
	case ICMP_SOURCE_QUENCH:
		printf("Source Quench\n");
		if (icp && (options & F_VERBOSE))
			pr_iph((struct iphdr*)(icp + 1));
		break;
	case ICMP_REDIRECT:
		switch(code) {
		case ICMP_REDIR_NET:
			printf("Redirect Network");
			break;
		case ICMP_REDIR_HOST:
			printf("Redirect Host");
			break;
		case ICMP_REDIR_NETTOS:
			printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIR_HOSTTOS:
			printf("Redirect Type of Service and Host");
			break;
		default:
			printf("Redirect, Bad Code: %d", code);
			break;
		}
		if (icp)
			printf("(New nexthop: %s)\n", pr_addr(icp->un.gateway));
		if (icp && (options & F_VERBOSE))
			pr_iph((struct iphdr*)(icp + 1));
		break;
	case ICMP_ECHO:
		printf("Echo Request\n");
		/* XXX ID + Seq + Data */
		break;
	case ICMP_TIME_EXCEEDED:
		switch(code) {
		case ICMP_EXC_TTL:
			printf("Time to live exceeded\n");
			break;
		case ICMP_EXC_FRAGTIME:
			printf("Frag reassembly time exceeded\n");
			break;
		default:
			printf("Time exceeded, Bad Code: %d\n", code);
			break;
		}
		if (icp && (options & F_VERBOSE))
			pr_iph((struct iphdr*)(icp + 1));
		break;
	case ICMP_PARAMETERPROB:
		printf("Parameter problem: pointer = %u\n", icp ? (ntohl(icp->un.gateway)>>24) : info);
		if (icp && (options & F_VERBOSE))
			pr_iph((struct iphdr*)(icp + 1));
		break;
	case ICMP_TIMESTAMP:
		printf("Timestamp\n");
		/* XXX ID + Seq + 3 timestamps */
		break;
	case ICMP_TIMESTAMPREPLY:
		printf("Timestamp Reply\n");
		/* XXX ID + Seq + 3 timestamps */
		break;
	case ICMP_INFO_REQUEST:
		printf("Information Request\n");
		/* XXX ID + Seq */
		break;
	case ICMP_INFO_REPLY:
		printf("Information Reply\n");
		/* XXX ID + Seq */
		break;
#ifdef ICMP_MASKREQ
	case ICMP_MASKREQ:
		printf("Address Mask Request\n");
		break;
#endif
#ifdef ICMP_MASKREPLY
	case ICMP_MASKREPLY:
		printf("Address Mask Reply\n");
		break;
#endif
	default:
		printf("Bad ICMP type: %d\n", type);
	}
}

void pr_options(unsigned char * cp, int hlen)
{
	int i, j;
	int optlen, totlen;
	unsigned char * optptr;
	static int old_rrlen;
	static char old_rr[MAX_IPOPTLEN];

	totlen = hlen-sizeof(struct iphdr);
	optptr = cp;

	while (totlen > 0) {
		if (*optptr == IPOPT_EOL)
			break;
		if (*optptr == IPOPT_NOP) {
			totlen--;
			optptr++;
			printf("\nNOP");
			continue;
		}
		cp = optptr;
		optlen = optptr[1];
		if (optlen < 2 || optlen > totlen)
			break;

		switch (*cp) {
		case IPOPT_SSRR:
		case IPOPT_LSRR:
			printf("\n%cSRR: ", *cp==IPOPT_SSRR ? 'S' : 'L');
			j = *++cp;
			i = *++cp;
			i -= 4;
			cp++;
			if (j > IPOPT_MINOFF) {
				for (;;) {
					__u32 address;
					memcpy(&address, cp, 4);
					cp += 4;
					if (address == 0)
						printf("\t0.0.0.0");
					else
						printf("\t%s", pr_addr(address));
					j -= 4;
					putchar('\n');
					if (j <= IPOPT_MINOFF)
						break;
				}
			}
			break;
		case IPOPT_RR:
			j = *++cp;		/* get length */
			i = *++cp;		/* and pointer */
			if (i > j)
				i = j;
			i -= IPOPT_MINOFF;
			if (i <= 0)
				continue;
			if (i == old_rrlen
			    && !bcmp((char *)cp, old_rr, i)) {
				printf("\t(same route)");
				i = ((i + 3) / 4) * 4;
				cp += i;
				break;
			}
			old_rrlen = i;
			bcopy((char *)cp, old_rr, i);
			printf("\nRR: ");
			cp++;
			for (;;) {
				__u32 address;
				memcpy(&address, cp, 4);
				cp += 4;
				if (address == 0)
					printf("\t0.0.0.0");
				else
					printf("\t%s", pr_addr(address));
				i -= 4;
				putchar('\n');
				if (i <= 0)
					break;
			}
			break;
		case IPOPT_TS:
		{
			int stdtime = 0, nonstdtime = 0;
			__u8 flags;
			j = *++cp;		/* get length */
			i = *++cp;		/* and pointer */
			if (i > j)
				i = j;
			i -= 5;
			if (i <= 0)
				continue;
			flags = *++cp;
			printf("\nTS: ");
			cp++;
			for (;;) {
				long l;

				if ((flags&0xF) != IPOPT_TS_TSONLY) {
					__u32 address;
					memcpy(&address, cp, 4);
					cp += 4;
					if (address == 0)
						printf("\t0.0.0.0");
					else
						printf("\t%s", pr_addr(address));
					i -= 4;
					if (i <= 0)
						break;
				}
				l = *cp++;
				l = (l<<8) + *cp++;
				l = (l<<8) + *cp++;
				l = (l<<8) + *cp++;

				if  (l & 0x80000000) {
					if (nonstdtime==0)
						printf("\t%ld absolute not-standard", l&0x7fffffff);
					else
						printf("\t%ld not-standard", (l&0x7fffffff) - nonstdtime);
					nonstdtime = l&0x7fffffff;
				} else {
					if (stdtime==0)
						printf("\t%ld absolute", l);
					else
						printf("\t%ld", l - stdtime);
					stdtime = l;
				}
				i -= 4;
				putchar('\n');
				if (i <= 0)
					break;
			}
			if (flags>>4)
				printf("Unrecorded hops: %d\n", flags>>4);
			break;
		}
		default:
			printf("\nunknown option %x", *cp);
			break;
		}
		totlen -= optlen;
		optptr += optlen;
	}
}


/*
 * pr_iph --
 *	Print an IP header with options.
 */
void pr_iph(struct iphdr *ip)
{
	int hlen;
	u_char *cp;

	hlen = ip->ihl << 2;
	cp = (u_char *)ip + 20;		/* point to options */

	printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst Data\n");
	printf(" %1x  %1x  %02x %04x %04x",
	       ip->version, ip->ihl, ip->tos, ip->tot_len, ip->id);
	printf("   %1x %04x", ((ip->frag_off) & 0xe000) >> 13,
	       (ip->frag_off) & 0x1fff);
	printf("  %02x  %02x %04x", ip->ttl, ip->protocol, ip->check);
	printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->saddr));
	printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->daddr));
	printf("\n");
	pr_options(cp, hlen);
}

/*
 * pr_addr --
 *	Return an ascii host address as a dotted quad and optionally with
 * a hostname.
 */
char *
pr_addr(__u32 addr)
{
	struct hostent *hp;
	static char buf[4096];
	static __u32 addr_cache = 0;

	if ( addr == addr_cache )
		return buf;

	addr_cache = addr;

	if ((options & F_NUMERIC) ||
	    !(hp = gethostbyaddr((char *)&addr, 4, AF_INET)))
		sprintf(buf, "%s", inet_ntoa(*(struct in_addr *)&addr));
	else
		snprintf(buf, sizeof(buf), "%s (%s)", hp->h_name,
			 inet_ntoa(*(struct in_addr *)&addr));
	return(buf);
}


void install_filter(void)
{
	static int once;
	static struct sock_filter insns[] = {
		BPF_STMT(BPF_LDX|BPF_B|BPF_MSH, 0), /* Skip IP header. F..g BSD... Look into ping6. */
		BPF_STMT(BPF_LD|BPF_H|BPF_IND, 4), /* Load icmp echo ident */
		BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, 0xAAAA, 0, 1), /* Ours? */
		BPF_STMT(BPF_RET|BPF_K, ~0U), /* Yes, it passes. */
		BPF_STMT(BPF_LD|BPF_B|BPF_IND, 0), /* Load icmp type */
		BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, ICMP_ECHOREPLY, 1, 0), /* Echo? */
		BPF_STMT(BPF_RET|BPF_K, 0xFFFFFFF), /* No. It passes. */
		BPF_STMT(BPF_RET|BPF_K, 0) /* Echo with wrong ident. Reject. */
	};
	static struct sock_fprog filter = {
		sizeof insns / sizeof(insns[0]),
		insns
	};

	if (once)
		return;
	once = 1;

	/* Patch bpflet for current identifier. */
	insns[2] = (struct sock_filter)BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, htons(ident), 0, 1);

	if (setsockopt(icmp_sock, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter)))
		perror("WARNING: failed to install socket filter\n");
}

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */