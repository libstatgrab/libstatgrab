# Makefile for libstatgrab/docs
# http://www.i-scream.org/
# $Id$

man_MANS = get_load_stats.3 get_general_stats.3 cpu_percent_usage.3 \
	   get_cpu_totals.3 get_cpu_diff.3 get_network_stats.3 \
	   get_network_stats_diff.3 get_diskio_stats.3 \
	   get_diskio_stats_diff.3 get_user_stats.3 get_page_stats.3 \
	   get_page_stats_diff.3 get_process_stats.3 \
	   get_memory_stats.3 get_swap_stats.3 statgrab.3

EXTRA_DIST = $(man_MANS)

%.3: %.xml
	docbook2man -N --symlinks $<

