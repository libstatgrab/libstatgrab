#
# i-scream central monitoring system
# http://www.i-scream.org
# Copyright (C) 2000-2004 i-scream
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# $Id$
#

ctypedef long time_t

cdef extern from "statgrab.h":
    ctypedef struct cpu_states_t:
        long long user
        long long kernel
        long long idle
        long long iowait
        long long swap
        long long nice
        long long total
        time_t systime

    ctypedef struct cpu_percent_t:
        float user
        float kernel
        float idle
        float iowait
        float swap
        float nice
        time_t time_taken

    ctypedef struct mem_stat_t:
        long long total
        long long free
        long long used
        long long cache

    ctypedef struct load_stat_t:
        double min1
        double min5
        double min15

    ctypedef struct user_stat_t:
        char *name_list
        int num_entries

    ctypedef struct swap_stat_t:
        long long total
        long long used
        long long free

    ctypedef struct general_stat_t:
        char *os_name
        char *os_release
        char *os_version
        char *platform
        char *hostname
        time_t uptime

    ctypedef struct disk_stat_t:
        char *device_name
        char *fs_type
        char *mnt_point
        long long size
        long long used
        long long avail
        long long total_inodes
        long long used_inodes
        long long free_inodes

    ctypedef struct diskio_stat_t:
        char *disk_name
        long long read_bytes
        long long write_bytes
        time_t systime

    ctypedef struct process_stat_t:
        int total
        int running
        int sleeping
        int stopped
        int zombie

    ctypedef struct network_stat_t:
        char *interface_name
        long long tx
        long long rx
        time_t systime

    ctypedef struct page_stat_t:
        long long pages_pagein
        long long pages_pageout
        time_t systime

    cdef extern cpu_states_t *get_cpu_totals()
    cdef extern cpu_states_t *get_cpu_diff()
    cdef extern cpu_percent_t *cpu_percent_usage()
    cdef extern mem_stat_t *get_memory_stats()
    cdef extern load_stat_t *get_load_stats()
    cdef extern user_stat_t *get_user_stats()
    cdef extern swap_stat_t *get_swap_stats()
    cdef extern general_stat_t *get_general_stats()
    cdef extern disk_stat_t *get_disk_stats(int *entries)
    cdef extern diskio_stat_t *get_diskio_stats(int *entries)
    cdef extern diskio_stat_t *get_diskio_stats_diff(int *entries)
    cdef extern process_stat_t *get_process_stats()
    cdef extern network_stat_t *get_network_stats(int *entries)
    cdef extern network_stat_t *get_network_stats_diff(int *entries)
    cdef extern page_stat_t *get_page_stats()
    cdef extern page_stat_t *get_page_stats_diff()
    cdef extern int statgrab_init()
    cdef extern int statgrab_drop_privileges()


class Result:
    def __init__(self, attrs):
        self.attrs = attrs
        for attr in attrs:
            setattr(self, attr, attrs[attr])
    def __getitem__(self, item):
        return getattr(self, item)
    def __repr__(self):
        return str(self.attrs)

class StatgrabException(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)


def py_get_cpu_totals():
    cdef cpu_states_t *s
    s = get_cpu_totals()
    if s == NULL:
        raise StatgrabException, 'get_cpu_totals() returned NULL'
    return Result(
        {'user': s.user,
         'kernel': s.kernel,
         'idle': s.idle,
         'iowait': s.iowait,
         'swap': s.swap,
         'nice': s.nice,
         'total': s.total,
         'systime': s.systime,
        }
    )

def py_get_cpu_diff():
    cdef cpu_states_t *s
    s = get_cpu_diff()
    if s == NULL:
        raise StatgrabException, 'get_cpu_diff() returned NULL'
    return Result(
        {'user': s.user,
         'kernel': s.kernel,
         'idle': s.idle,
         'iowait': s.iowait,
         'swap': s.swap,
         'nice': s.nice,
         'total': s.total,
         'systime': s.systime,
        }
    )

def py_cpu_percent_usage():
    cdef cpu_percent_t *s
    s = cpu_percent_usage()
    if s == NULL:
        raise StatgrabException, 'cpu_percent_usage() returned NULL'
    return Result(
        {'user': s.user,
         'kernel': s.kernel,
         'idle': s.idle,
         'iowait': s.iowait,
         'swap': s.swap,
         'nice': s.nice,
         'time_taken': s.time_taken,
        }
    )

def py_get_memory_stats():
    cdef mem_stat_t *s
    s = get_memory_stats()
    if s == NULL:
        raise StatgrabException, 'get_memory_stats() returned NULL'
    return Result(
        {'total': s.total,
         'used': s.used,
         'free': s.free,
         'cache': s.cache,
        }
    )

def py_get_load_stats():
    cdef load_stat_t *s
    s = get_load_stats()
    if s == NULL:
        raise StatgrabException, 'get_load_stats() returned NULL'
    return Result(
        {'min1': s.min1,
         'min5': s.min5,
         'min15': s.min15,
        }
    )

def py_get_user_stats():
    cdef user_stat_t *s
    s = get_user_stats()
    if s == NULL:
        raise StatgrabException, 'get_user_stats() returned NULL'
    return Result(
        {'name_list': s.name_list,
         'num_entries': s.num_entries,
        }
    )

def py_get_swap_stats():
    cdef swap_stat_t *s
    s = get_swap_stats()
    if s == NULL:
        raise StatgrabException, 'get_swap_stats() returned NULL'
    return Result(
        {'total': s.total,
         'used': s.used,
         'free': s.free,
        }
    )

def py_get_general_stats():
    cdef general_stat_t *s
    s = get_general_stats()
    if s == NULL:
        raise StatgrabException, 'get_general_stats() returned NULL'
    return Result(
        {'os_name': s.os_name,
         'os_release': s.os_release,
         'os_version': s.os_version,
         'platform': s.platform,
         'hostname': s.hostname,
         'uptime': s.uptime,
        }
    )

def py_get_disk_stats():
    cdef disk_stat_t *s
    cdef int entries
    s = get_disk_stats(&entries)
    if s == NULL:
        raise StatgrabException, 'get_disk_stats() returned NULL'
    list = [entries]
    for i from 0 <= i < entries:
        list.append(Result(
            {'device_name': s.device_name,
             'fs_type': s.fs_type,
             'mnt_point': s.mnt_point,
             'size': s.size,
             'used': s.used,
             'avail': s.avail,
             'total_inodes': s.total_inodes,
             'used_inodes': s.used_inodes,
             'free_inodes': s.free_inodes,
            }
        ))
        s = s + 1
    return list

def py_get_diskio_stats():
    cdef diskio_stat_t *s
    cdef int entries
    s = get_diskio_stats(&entries)
    if s == NULL:
        raise StatgrabException, 'get_diskio_stats() returned NULL'
    list = [entries]
    for i from 0 <= i < entries:
        list.append(Result(
            {'disk_name': s.disk_name,
             'read_bytes': s.read_bytes,
             'write_bytes': s.write_bytes,
             'systime': s.systime,
            }
        ))
        s = s + 1
    return list

def py_get_diskio_stats_diff():
    cdef diskio_stat_t *s
    cdef int entries
    s = get_diskio_stats_diff(&entries)
    if s == NULL:
        raise StatgrabException, 'get_diskio_stats_diff() returned NULL'
    list = [entries]
    for i from 0 <= i < entries:
        list.append(Result(
            {'disk_name': s.disk_name,
             'read_bytes': s.read_bytes,
             'write_bytes': s.write_bytes,
             'systime': s.systime,
            }
        ))
        s = s + 1
    return list

def py_get_process_stats():
    cdef process_stat_t *s
    s = get_process_stats()
    if s == NULL:
        raise StatgrabException, 'get_process_stats() returned NULL'
    return Result(
        {'total': s.total,
         'running': s.running,
         'sleeping': s.sleeping,
         'stopped': s.stopped,
         'zombie': s.zombie,
        }
    )

def py_get_network_stats():
    cdef network_stat_t *s
    cdef int entries
    s = get_network_stats(&entries)
    if s == NULL:
        raise StatgrabException, 'get_network_stats() returned NULL'
    list = [entries]
    for i from 0 <= i < entries:
        list.append(Result(
            {'interface_name': s.interface_name,
             'tx': s.tx,
             'rx': s.rx,
             'systime': s.systime,
            }
        ))
        s = s + 1
    return list

def py_get_network_stats_diff():
    cdef network_stat_t *s
    cdef int entries
    s = get_network_stats_diff(&entries)
    if s == NULL:
        raise StatgrabException, 'get_network_stats_diff() returned NULL'
    list = [entries]
    for i from 0 <= i < entries:
        list.append(Result(
            {'interface_name': s.interface_name,
             'tx': s.tx,
             'rx': s.rx,
             'systime': s.systime,
            }
        ))
        s = s + 1
    return list

def py_get_page_stats():
    cdef page_stat_t *s
    s = get_page_stats()
    if s == NULL:
        raise StatgrabException, 'get_page_stats() returned NULL'
    return Result(
        {'pages_pagein': s.pages_pagein,
         'pages_pageout': s.pages_pageout,
        }
    )

def py_get_page_stats_diff():
    cdef page_stat_t *s
    s = get_page_stats_diff()
    if s == NULL:
        raise StatgrabException, 'get_page_stats_diff() returned NULL'
    return Result(
        {'pages_pagein': s.pages_pagein,
         'pages_pageout': s.pages_pageout,
        }
    )

def py_statgrab_init():
    return statgrab_init()

def py_statgrab_drop_privileges():
    return statgrab_drop_privileges()
