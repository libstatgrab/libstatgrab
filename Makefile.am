# Makefile for libstatgrab
# http://www.i-scream.org/libstatgrab/
# $Id$

AUTOMAKE_OPTIONS = gnu
ACLOCAL_AMFLAGS = -I m4

DOCS_SUBDIR = docs

if TESTS
TESTS_SUBDIR = tests
endif

if BUILD_WITH_GETTEXT
SUBDIRS = po src examples $(DOCS_SUBDIR) $(TESTS_SUBDIR)
else
SUBDIRS = src examples $(DOCS_SUBDIR) $(TESTS_SUBDIR)
endif

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

EXTRA_DIST = libstatgrab.pc.in PLATFORMS COPYING COPYING.LGPL

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libstatgrab.pc
