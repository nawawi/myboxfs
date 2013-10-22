# Comment/uncomment the following line to disable/enable debugging
DEBUG = y

# This Makefile has been simplified as much as possible, by putting all
# generic material, independent of this specific directory, into
# ../Rules.make. Read that file for details

TOPDIR  := $(shell cd ..;pwd)
include $(TOPDIR)/Rules.make

# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

CFLAGS += $(DEBFLAGS) -DLINUX_24
CFLAGS += -I.

TARGET = wdt.o
OBJS = $(TARGET).o
SRC = w83627hf_wdt.o

all: .depend $(TARGET)

$(TARGET): $(SRC)
	$(LD) -s  -r $^ -o $@ 

install:
	install -d $(INSTALLDIR)
	install -c $(TARGET) $(INSTALLDIR)

clean:
	rm -f *.o *~ core .depend 

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > $@


ifeq (.depend,$(wildcard .depend))
include .depend
endif
