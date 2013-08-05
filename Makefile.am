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

test:
if TEST_SCRIPTS
	cd tests && $(MAKE) test
else
	@echo Not built
endif

check: test

EXTRA_DIST = libstatgrab.pc.in PLATFORMS COPYING COPYING.LGPL

MAINTAINERCLEANFILES = ChangeLog

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libstatgrab.pc
