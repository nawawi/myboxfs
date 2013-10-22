# Generated automatically from Makefile.in by configure.
#
#  Makefile for the Squid filtering modules.
#  Author: Olaf Titz
#

prefix		= /
exec_prefix	= ${prefix}
exec_suffix	= @exec_suffix@
cgi_suffix	= @cgi_suffix@
top_srcdir	= ..
bindir		= /bin
libexecdir	= ${exec_prefix}/libexec
sysconfdir	= /etc/cf
localstatedir	= ${prefix}/var
srcdir		= .

CC		= gcc -g -Wall
INSTALL		= /usr/bin/install -c
INSTALL_BIN	= ${INSTALL}
INSTALL_FILE	= ${INSTALL} -m 644
INSTALL_SUID	= ${INSTALL} -o root -m 4755
RANLIB		= ranlib
LN_S		= ln -s
PERL		= /usr/bin/perl
AC_CFLAGS	= -m32 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -g -Wall -D_REENTRANT
LDFLAGS		= -Wl,-E -m32 -g
SHELL		= /bin/sh
LD_SH		= gcc -shared -fpic

INCLUDE		= -I. -I../include -I../src -I$(top_srcdir)/include
CFLAGS		= $(AC_CFLAGS) $(CFLAGS_SH) $(INCLUDE) $(DEFINES) $(CPPFLAGS)

MODULES=cfmod_redirect.so cfmod_cookies.so cfmod_htmlfilter.so cfmod_script.so cfmod_virus.so \
	cfmod_image.so cfmod_objects.so cfmod_rejecttype.so cfmod_allowtype.so

OBJS=$(MODULES:.so=.o)

all:	$(MODULES)

# needeed for HPUX's broken make
$(MODULES): $(OBJS)

$(OBJS): classes.dh

CLDEPS=$(MODULES:.so=.c) htmlfilter.h \
	../src/module.c ../src/module.h  ../src/patfile.c ../src/patfile.h

classes.dh: $(CLDEPS) classdef
	$(PERL) classdef -sC -o classes $(CLDEPS)

install: all
	for i in $(MODULES); do \
		mv -f $(libexecdir)/$$i $(libexecdir)/$$i- 2>/dev/null; \
		$(INSTALL_BIN) $$i $(libexecdir); \
		rm -f $(libexecdir)/$$i- 2>/dev/null; \
	done
	$(INSTALL_FILE) redirect.sample $(sysconfdir)

# This is known to work only with gperf version 2.7.
# We make an extra rule here instead of a direct dependency
# in order to avoid unnecessary/accidental remake.
jsattrib.c.new: Makefile jsattrib.gperf
	gperf -k'1,5,$$' -o -l -C -NisJSAttrib <jsattrib.gperf |\
	 sed -e 's:(unsigned char)str\[[^]]*\]:tolower(\0):g' \
	     -e 's:\*\([a-z]*\) *== *\*\([a-z]*\) *&& *\!strcmp *(\1 *+ *1, *\2 *+ *1):!strcasecmp(\1, \2):g' \
	>jsattrib.c.new

jsre.h: Makefile makejsre
	$(PERL) makejsre mocha javascript livescript eval >jsre.h

.SUFFIXES: .c .o .so

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.o.so:
	$(LD_SH) $(LDFLAGS_SH) -o $@ $< -lc

clean:
	-rm -rf *.o *.so *pure_* core *.out *.s classes.th classes.dh jsre.h

distclean: clean
	-rm -f Makefile *~ *.bak *.new

cfmod_script.o: ../src/squid.h ../src/module.h htmlfilter.h jsattrib.c jsre.h

