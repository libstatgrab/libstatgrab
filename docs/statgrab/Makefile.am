# Makefile for libstatgrab/docs/statgrab
# http://www.i-scream.org/libstatgrab/
# $Id$

if MANPAGES
if STATGRAB
man_MANS = statgrab.1 statgrab-make-mrtg-config.1 \
	   statgrab-make-mrtg-index.1
endif
endif

EXTRA_DIST = $(man_MANS)

%.1: %.xml
	docbook2man -N --symlinks $<

