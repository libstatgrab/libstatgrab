# Makefile for libstatgrab/src/statgrab
# http://www.i-scream.org/
# $Id$

bin_PROGRAMS = statgrab
statgrab_SOURCES = statgrab.c

INCLUDES = -I$(top_srcdir) -I$(top_builddir) \
           -I$(top_srcdir)/src -I$(top_builddir)/src\
           -I$(top_srcdir)/src/libstatgrab -I$(top_builddir)/src/libstatgrab

LDADD = $(top_builddir)/src/libstatgrab/libstatgrab.la
