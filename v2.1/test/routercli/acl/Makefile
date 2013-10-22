# Copyright (c) 2004 Alexander Eremin <xyzyx@rambler.ru>
#
CFLAGS  = -Wall -Wunused -DNETFILTER_VERSION=\"1.2.9\" 
all: acl.o    
acl.o: acl.c 
	$(MAKE) -C libiptc/ all
	gcc $(CFLAGS) -c acl.c 

   
