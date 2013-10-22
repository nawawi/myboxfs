#lease modify the KSRC to satisfy your system
KSRC=/usr/src/linux-2.6

CFILES = main.c
TARGET = panic.ko

EXTRA_CFLAGS += $(CFLAGS_EXTRA)
obj-m += panic.o
panic-objs := $(CFILES:.c=.o)
default: 
	make -C $(KSRC) SUBDIRS=$(shell pwd) modules

clean:
	rm -f *.o *.ko *.mod.c
