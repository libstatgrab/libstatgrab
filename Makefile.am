# Makefile for libstatgrab
# http://www.i-scream.org/libstatgrab/
# $Id$

AUTOMAKE_OPTIONS = gnu
ACLOCAL_AMFLAGS = -I m4

DOCS_SUBDIR = docs

if TESTS
TESTS_SUBDIR = tests
endif

SUBDIRS = src examples $(DOCS_SUBDIR) $(TESTS_SUBDIR)

# FIXME - this needs updating to use Git?
ChangeLog:
	cvs2cl --stdout --no-wrap -r -t -w -S -g "-d :pserver:anonymous@cvs.i-scream.org:/cvs/i-scream" -g "-q" > ChangeLog

test:
if TEST_SCRIPTS
	cd tests && $(MAKE) test
else
	@echo Not built
endif

EXTRA_DIST = libstatgrab.pc.in PLATFORMS COPYING COPYING.LGPL

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libstatgrab.pc
