# Makefile for libstatgrab/docs/statgrab
# https://libstatgrab.org/
# $Id$

if MANPAGES
if STATGRAB
man_MANS = statgrab.1 statgrab-make-mrtg-config.1 \
	   statgrab-make-mrtg-index.1
endif

EXTRA_DIST = $(man_MANS)

MAINTAINERCLEANFILES= $(man_MANS)

.xml.1:
	$(DOCBOOK2MAN) -N --symlinks --string-param \
		header-3="`git log -1 --format='%ad' --date=short $<`" \
		$<
endif
