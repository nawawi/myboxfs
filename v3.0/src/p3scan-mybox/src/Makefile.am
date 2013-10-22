bin_PROGRAMS = p3scan
p3scan_SOURCES = p3scan.c getlinep3.c parsefile.c \
	scanner_clamd.c scanner_bash.c scanner_basic.c \
	scanner_sophie.c restart.c

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
p3scan_LDFLAGS = $(all_libraries) 
EXTRA_HEADERS = 
noinst_HEADERS = getlinep3.h p3scan.h parsefile.h scanner.h  \
		restart.h


