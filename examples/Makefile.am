# Makefile for libstatgrab/examples
# http://www.i-scream.org/libstatgrab/
# $Id$

if EXAMPLES
noinst_PROGRAMS = network_traffic disk_traffic cpu_usage load_stats process_stats os_info vm_stats user_list page_stats
endif

INCLUDES = -I$(top_srcdir) -I$(top_builddir)\
           -I$(top_srcdir)/src -I$(top_builddir)/src\
           -I$(top_srcdir)/src/libstatgrab -I$(top_builddir)/src/libstatgrab

LDADD = $(top_builddir)/src/libstatgrab/libstatgrab.la @LINKFLAGS@
