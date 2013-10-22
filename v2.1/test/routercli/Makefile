# Copyright (c) 2004 Alexander Eremin <xyzyx@rambler.ru>
# 
# Makefile for routercli 2004-07-09
VERSION = 0.4
# Config directory - don`t touch
CONFDIR = /etc/routercli
# Program directory - you can change this as want
RCLIDIR = /usr/sbin 
######################################################################################

CC = gcc
CFLAGS  = -Wall -O2 -D_GNU_SOURCE 
ACLDIR = acl
ACLOBJS = $(ACLDIR)/acl.o
RIPDIR = rip
ACLLIB = $(ACLDIR)/libiptc
ACLFLAGS = $(ACLLIB)/libiptc.a
CLIDIR = cli
LDFLAGS = -lreadline -lcurses -lcrypt -rdynamic $(ACLFLAGS) -ldl
PROGS = routercli
all: $(PROGS)

CLIOBJS = $(CLIDIR)/main.o $(CLIDIR)/ipcalc.o $(CLIDIR)/ifshow.o \
$(CLIDIR)/hostname.o $(CLIDIR)/telnet.o $(CLIDIR)/ifconfig.o $(CLIDIR)/makeargv.o\
$(CLIDIR)/route.o $(CLIDIR)/util.o $(CLIDIR)/conf.o $(CLIDIR)/netutil.o \
$(CLIDIR)/trace.o $(CLIDIR)/ping.o $(CLIDIR)/wildcard.o 

RIPOBJS = $(RIPDIR)/af.o $(RIPDIR)/if.o $(RIPDIR)/input.o $(RIPDIR)/ripd.o \
$(RIPDIR)/output.o $(RIPDIR)/startup.o $(RIPDIR)/tables.o $(RIPDIR)/timer.o \
$(RIPDIR)/trace.o $(RIPDIR)/inet.o
    
routercli: $(CLIOBJS) 
	$(MAKE) -C $(RIPDIR) all
	$(MAKE) -C $(ACLDIR) all
	$(CC) -o $@ $(CFLAGS) $(CLIOBJS) $(ACLOBJS) $(RIPOBJS) $(LDFLAGS)

install: routercli
	 mkdir $(CONFDIR); mv routercli $(RCLIDIR);
	 cp cli.conf.sample $(CONFDIR);cp ripd.conf $(CONFDIR);
	 make clean; 
	 @echo "+---------------------------------------------------------------------------+";
	 @echo
	 @echo "        RouterCli $(VERSION) by Alexander Eremin <xyzyx@rambler.ru>";
	 @echo   
	 @echo "	Now you can run routercli from $(RCLIDIR) directory";
	 @echo "	See INSTALL for details";
	 @echo  
	 @echo "+---------------------------------------------------------------------------+";
clean:
	rm -f *.o *~ core
	rm -f cli/*.o
	rm -f acl/libiptc/*.o *.a
	rm -f acl/*.o 
	rm -f rip/*.o 


