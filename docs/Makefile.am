# Makefile for libstatgrab/docs
# http://www.i-scream.org/
# $Id$

DOCS = network.txt diskio.txt load_average.txt process_stats.txt os_stats.txt vm_stats.txt user_list.txt cpu.txt page_stat.txt

man_MANS = get_load_stats.3 get_general_stats.3 cpu_percent_usage.3 \
	   get_cpu_totals.3 get_cpu_diff.3 get_network_stats.3 \
	   get_network_stats_diff.3 get_diskio_stats.3 \
	   get_diskio_stats_diff.3 get_user_stats.3

docdir = $(datadir)/doc/libstatgrab

doc_DATA = $(DOCS)
EXTRA_DIST = $(DOCS) $(man_MANS)

%.3: %.xml
	docbook2man -N --symlinks $<

