# Makefile for libstatgrab/src/saidar
# http://www.i-scream.org/
# $Id$

if SAIDAR
bin_PROGRAMS = saidar
endif

saidar_SOURCES = saidar.c

INCLUDES = @SAIDARCPPFLAGS@ -I$(top_srcdir) -I$(top_builddir) \
           -I$(top_srcdir)/src -I$(top_builddir)/src\
           -I$(top_srcdir)/src/libstatgrab -I$(top_builddir)/src/libstatgrab

LDADD = $(top_builddir)/src/libstatgrab/libstatgrab.la @SAIDARLIBS@ @LINKFLAGS@

if SETGIDBINS
# Some operating systems need the binary to be setgid kmem
install-exec-local:
	chgrp kmem @bindir@/saidar
	chmod g+s @bindir@/saidar
else
install-exec-local:
endif
