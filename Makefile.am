# Makefile for libstatgrab
# http://www.i-scream.org.uk/
# $Id$

AUTOMAKE_OPTIONS = gnu

lib_LIBRARIES = libstatgrab.a
libstatgrab_a_SOURCES = cpu_stats.c disk_stats.c load_stats.c memory_stats.c network_stats.o os_info.c process_stats.c swap_stats.c user_stats.c
include_HEADERS = statgrab.h

ChangeLog:
	cvs2cl --stdout --no-wrap -r -t -w -S -g "-d :pserver:anonymous@cvs.i-scream.org.uk:/cvs/i-scream" -g "-q" > ChangeLog
