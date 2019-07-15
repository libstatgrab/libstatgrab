/*
 * libstatgrab
 * https://libstatgrab.org
 * Copyright (C) 2003-2004 Peter Saunders
 * Copyright (C) 2003-2019 Tim Bishop
 * Copyright (C) 2003-2013 Adam Sampson
 * Copyright (C) 2012-2019 Jens Rehsack
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifdef WIN32

typedef enum {
	SG_WIN32_PROC_USER,
	SG_WIN32_PROC_PRIV,
	SG_WIN32_PROC_IDLE,
	SG_WIN32_PROC_INT,
	SG_WIN32_MEM_CACHE,
	SG_WIN32_UPTIME,
	SG_WIN32_PAGEIN,
	SG_WIN32_PAGEOUT,
	SG_WIN32_SIZE /* Leave this as last entry */
} pdh_enum;

#define PDH_USER "\\Processor(_Total)\\% User Time"
#define PDH_PRIV "\\Processor(_Total)\\% Privileged Time"
#define PDH_IDLE "\\Processor(_Total)\\% Processor Time"
#define PDH_INTER "\\Processor(_Total)\\% Interrupt Time"
#define PDH_DISKIOREAD "\\PhysicalDisk(%s)\\Disk Read Bytes/sec"
#define PDH_DISKIOWRITE "\\PhysicalDisk(%s)\\Disk Write Bytes/sec"
#define PDH_MEM_CACHE "\\Memory\\Cache Bytes"
#define PDH_UPTIME "\\System\\System Up Time"
#define PDH_PAGEIN "\\Memory\\Pages Input/sec"
#define PDH_PAGEOUT "\\Memory\\Pages Output/sec"

int add_counter(const char *fullCounterPath, HCOUNTER *phCounter);
int read_counter_double(pdh_enum counter, double *result);
int read_counter_large(pdh_enum counter, long long *result);
char *get_diskio(const int no, long long *read, long long *write);
int get_netio(const int no, char **name, long long *read, long long *write);

int sg_win32_snapshot();
int sg_win32_start_capture();
void sg_win32_end_capture();

#endif
