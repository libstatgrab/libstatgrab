# Makefile for libstatgrab/docs/saidar
# https://libstatgrab.org/

if MANPAGES
if SAIDAR
man_MANS = saidar.1
endif

EXTRA_DIST = $(man_MANS)

MAINTAINERCLEANFILES= $(man_MANS)

.xml.1:
	$(DOCBOOK2MAN) -N --symlinks --string-param \
		header-3="`git log -1 --format='%ad' --date=short $<`" \
		$<
endif
