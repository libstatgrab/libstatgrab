# Makefile for libstatgrab/docs
# http://www.i-scream.org/libstatgrab/
# $Id$

if MANPAGES
man_MANS = sg_get_cpu_stats.3 sg_get_cpu_stats_diff.3 \
	   sg_get_cpu_percents.3 sg_get_disk_io_stats.3 \
	   sg_get_disk_io_stats_diff.3 sg_get_host_info.3 \
	   sg_get_fs_stats.3 sg_get_load_stats.3 \
	   sg_get_mem_stats.3 sg_get_swap_stats.3 \
	   sg_get_network_iface_stats.3 sg_get_network_io_stats.3 \
	   sg_get_network_io_stats_diff.3 sg_get_page_stats.3 \
	   sg_get_page_stats_diff.3 sg_get_process_stats.3 \
	   sg_get_process_count.3 sg_get_user_stats.3 statgrab.3
endif

EXTRA_DIST = $(man_MANS)

%.3: %.xml
	docbook2man -N --symlinks $<

