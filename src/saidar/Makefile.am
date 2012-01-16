# Makefile for libstatgrab/src/saidar
# http://www.i-scream.org/libstatgrab/
# $Id$

if SAIDAR
bin_PROGRAMS = saidar
endif

saidar_SOURCES = saidar.c

AM_CPPFLAGS = @SAIDARCPPFLAGS@ -I$(top_srcdir) -I$(top_builddir) \
              -I$(top_srcdir)/src -I$(top_builddir)/src \
              -I$(top_srcdir)/src/libstatgrab -I$(top_builddir)/src/libstatgrab \
              @CLIBFLAGS@

LDADD = $(top_builddir)/src/libstatgrab/libstatgrab.la @SAIDARLIBS@ @LINKFLAGS@

install-exec-local:
if SAIDAR
if SETGIDBINS
# Some operating systems need the binary to be setgid kmem
	chgrp kmem $(DESTDIR)@bindir@/saidar
	chmod g+s $(DESTDIR)@bindir@/saidar
endif
if SETUIDBINS
# Some operating systems need the binary to be setuid root
	chown root $(DESTDIR)@bindir@/saidar
	chmod u+s $(DESTDIR)@bindir@/saidar
endif
endif
