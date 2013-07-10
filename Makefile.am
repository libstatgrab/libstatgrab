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

ChangeLog:
	git log --stat --name-only --date=short \
	--format='%n*%ad  %an  <%ae>%n%n%w(76,4,4)%s%n%n%b%n#Hash: %H%n#Files affected:' \
	| sed \
		-e 's/^\([^ \*]\)/    \1/' \
		-e 's/^    \#/  /' \
		-e 's/     Hash/    Hash/' \
		-e 's/^\*//' \
	| tail -n +2 > ChangeLog

test:
if TEST_SCRIPTS
	cd tests && $(MAKE) test
else
	@echo Not built
endif

check: test

EXTRA_DIST = libstatgrab.pc.in PLATFORMS COPYING COPYING.LGPL

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libstatgrab.pc
