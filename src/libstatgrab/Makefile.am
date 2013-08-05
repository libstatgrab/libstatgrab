# Makefile for libstatgrab/src/libstatgrab
# http://www.i-scream.org/libstatgrab/
# $Id$

AM_CPPFLAGS = $(PTHREAD_CFLAGS) -DBUILD_LIBSTATGRAB @CLIBFLAGS@

include_HEADERS = statgrab.h
lib_LTLIBRARIES = libstatgrab.la

libstatgrab_la_LDFLAGS = -version-info 9:0:0 @LINKFLAGS@

if MINGW
libstatgrab_la_MINGWSOURCES = win32.c
endif
libstatgrab_la_SOURCES =	cpu_stats.c disk_stats.c load_stats.c memory_stats.c network_stats.c os_info.c page_stats.c process_stats.c swap_stats.c user_stats.c \
				tools.c vector.c error.c globals.c $(libstatgrab_la_MINGWSOURCES)

noinst_HEADERS = error.h globals.h trace.h tools.h vector.h win32.h

EXTRA_DIST = statgrab.h.in
