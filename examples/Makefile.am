# Makefile for libstatgrab/examples
# http://www.i-scream.org/libstatgrab/
# $Id$

if EXAMPLES
noinst_PROGRAMS = network_traffic disk_traffic cpu_usage load_stats \
		  process_stats os_info vm_stats user_list page_stats \
		  network_iface_stats process_snapshot filesys_snapshot \
		  valid_filesystems
endif

# BUILT_HEADERS = helpers.h
EXTRA_network_traffic_SOURCES = helpers.c helpers.h

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)\
	      -I$(top_srcdir)/src -I$(top_builddir)/src\
	      -I$(top_srcdir)/src/libstatgrab -I$(top_builddir)/src/libstatgrab \
	       @CLIBFLAGS@

LDADD = $(top_builddir)/src/libstatgrab/libstatgrab.la helpers.o @LINKFLAGS@
