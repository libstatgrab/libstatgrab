# Makefile for libstatgrab/docs
# http://www.i-scream.org/
# $Id$

DOCS = network.txt diskio.txt load_average.txt process_stats.txt os_stats.txt vm_stats.txt user_list.txt cpu.txt page_stat.txt

man_MANS = get_load_stats.3 get_general_stats.3

docdir = $(datadir)/doc/libstatgrab

doc_DATA = $(DOCS)
EXTRA_DIST = $(DOCS) $(man_MANS)

%.3: %.xml
	docbook2man -N --symlinks $<

