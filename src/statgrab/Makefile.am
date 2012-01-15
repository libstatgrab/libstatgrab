# Makefile for libstatgrab/src/statgrab
# http://www.i-scream.org/libstatgrab/
# $Id$

if STATGRAB
bin_PROGRAMS = statgrab
bin_SCRIPTS = statgrab-make-mrtg-config statgrab-make-mrtg-index
endif

statgrab_SOURCES = statgrab.c

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir) \
              -I$(top_srcdir)/src -I$(top_builddir)/src\
              -I$(top_srcdir)/src/libstatgrab -I$(top_builddir)/src/libstatgrab \
	       @CLIBFLAGS@

LDADD = $(top_builddir)/src/libstatgrab/libstatgrab.la @LINKFLAGS@

install-exec-local:
if STATGRAB
if SETGIDBINS
# Some operating systems need the binary to be setgid kmem
	chgrp kmem $(DESTDIR)@bindir@/statgrab
	chmod g+s $(DESTDIR)@bindir@/statgrab
endif
if SETUIDBINS
# Some operating systems need the binary to be setuid root
	chown root $(DESTDIR)@bindir@/statgrab
	chmod u+s $(DESTDIR)@bindir@/statgrab
endif
endif
