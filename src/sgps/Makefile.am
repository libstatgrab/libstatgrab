# Makefile for libstatgrab/src/sgps
# http://www.i-scream.org/libstatgrab/
# $Id$

if SGPS
bin_PROGRAMS = sgps
endif

sgps_SOURCES = sgps.c

INCLUDES = -I$(top_srcdir) -I$(top_builddir) \
           -I$(top_srcdir)/src -I$(top_builddir)/src\
           -I$(top_srcdir)/src/libstatgrab -I$(top_builddir)/src/libstatgrab

LDADD = $(top_builddir)/src/libstatgrab/libstatgrab.la @LINKFLAGS@

install-exec-local:
if SGPS
if SETGIDBINS
# Some operating systems need the binary to be setgid kmem
	chgrp kmem $(DESTDIR)@bindir@/sgps
	chmod g+s $(DESTDIR)@bindir@/sgps
endif
if SETUIDBINS
# Some operating systems need the binary to be setuid root
	chown root $(DESTDIR)@bindir@/sgps
	chmod u+s $(DESTDIR)@bindir@/sgps
endif
endif
