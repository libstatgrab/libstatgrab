/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2004 i-scream
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include "statgrab.h"
#include "vector.h"
#include "tools.h"
#include <time.h>
#ifdef SOLARIS
#include <kstat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/sockio.h>
#include <unistd.h>
#endif
#ifdef LINUX
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <ctype.h>
#include <linux/version.h>
#include <asm/types.h>
/* These aren't defined by asm/types.h unless the kernel is being
   compiled, but some versions of ethtool.h need them. */
typedef __uint8_t u8;
typedef __uint16_t u16;
typedef __uint32_t u32;
typedef __uint64_t u64;
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <unistd.h>
#endif
#ifdef ALLBSD
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_media.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#ifdef AIX
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <errno.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <libperfstat.h>
#endif
#ifdef HPUX
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <errno.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/stropts.h>
#include <sys/dlpi.h>
#include <sys/dlpi_ext.h>
#include <sys/mib.h>
#define HP_UX11 /* Currently no HP-UX below 11 is supported, fix here if this changes */
#endif
#ifdef WIN32
#include <windows.h>
#include <Iphlpapi.h>
#include "win32.h"
#endif

static void network_stat_init(sg_network_io_stats *s) {
	s->interface_name = NULL;
	s->tx = 0;
	s->rx = 0;
	s->ipackets = 0;
	s->opackets = 0;
	s->ierrors = 0;
	s->oerrors = 0;
	s->collisions = 0;
}

static void network_stat_destroy(sg_network_io_stats *s) {
	free(s->interface_name);
}

VECTOR_DECLARE_STATIC(network_stats, sg_network_io_stats, 5,
		      network_stat_init, network_stat_destroy);

#ifdef WIN32
static PMIB_IFTABLE win32_get_devices()
{
	PMIB_IFTABLE if_table;
	PMIB_IFTABLE tmp;
	unsigned long dwSize = 0;

	// Allocate memory for pointers
	if_table = sg_malloc(sizeof(MIB_IFTABLE));
	if(if_table == NULL) {
		return NULL;
	}

	// Get necessary size for the buffer
	if(GetIfTable(if_table, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
		tmp = sg_realloc(if_table, dwSize);
		if(tmp == NULL) {
			free(if_table);
			return NULL;
		}
		if_table = tmp;
	}

	// Get the data
	if(GetIfTable(if_table, &dwSize, 0) != NO_ERROR) {
		free(if_table);
		return NULL;
	}
	return if_table;
}
#endif /* WIN32 */

sg_network_io_stats *sg_get_network_io_stats(int *entries){
	int interfaces;
#ifndef AIX
	sg_network_io_stats *network_stat_ptr;
#endif
#ifdef HPUX
/* 
 * talk to Data Link Provider Interface aka /dev/dlpi
 * see: http://docs.hp.com/hpux/onlinedocs/B2355-90139/B2355-90139.html
 */
#define BUF_SIZE 40960

	u_long *ctrl_area = malloc(BUF_SIZE);
	u_long *data_area = malloc(BUF_SIZE);
	u_long *ppa_area = malloc(BUF_SIZE);
	struct strbuf ctrl_buf = {BUF_SIZE, 0, (char*) ctrl_area};
	struct strbuf data_buf = {BUF_SIZE, 0, (char*) data_area};

	dl_hp_ppa_info_t *ppa_info;
	dl_hp_ppa_req_t *ppa_req;
	dl_hp_ppa_ack_t *ppa_ack;
	char name_buf[24];
	int fd = -1, ppa_count, count, flags;
#endif

#ifdef SOLARIS
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_named_t *knp;
#endif

#ifdef LINUX
	FILE *f;
	/* Horrible big enough, but it should be easily big enough */
	char line[8096];
	regex_t regex;
	regmatch_t line_match[9];
#endif
#ifdef ALLBSD
	struct ifaddrs *net, *net_ptr;
	struct if_data *net_data;
#endif
#ifdef WIN32
	PMIB_IFTABLE if_table;
	MIB_IFROW if_row;
	int i, no, j;

	/* used for duplicate interface names. 5 for space, hash, up to two
	 * numbers and terminating slash */
	char buf[5];
#endif
#ifdef AIX
	int i;
	perfstat_netinterface_t *statp;
	perfstat_id_t first;
#endif

#ifdef HPUX
	interfaces=0;

	if ((fd = open("/dev/dlpi", O_RDWR)) < 0) {
		sg_set_error_with_errno(SG_ERROR_OPEN, "/dev/dlpi");
		return NULL;
	}

	ppa_req = (dl_hp_ppa_req_t *) ctrl_area;
	ppa_ack = (dl_hp_ppa_ack_t *) ctrl_area;

	ppa_req->dl_primitive = DL_HP_PPA_REQ;

	ctrl_buf.len = sizeof(dl_hp_ppa_req_t);
	if(putmsg(fd, &ctrl_buf, 0, 0) < 0) {
		sg_set_error_with_errno(SG_ERROR_PUTMSG, "DL_HP_PPA_REQ");
		return NULL;
	}

	flags = 0;
	ctrl_area[0] = 0;

	if (getmsg(fd, &ctrl_buf, &data_buf, &flags) < 0) {
		sg_set_error_with_errno(SG_ERROR_GETMSG, "DL_HP_PPA_REQ");
		return NULL;
	}

	if (ppa_ack->dl_length == 0) {
		sg_set_error_with_errno(SG_ERROR_PARSE, "DL_HP_PPA_REQ");
		return NULL;
	}

	// save all the PPA information
	memcpy ((u_char*) ppa_area, (u_char*) ctrl_area + ppa_ack->dl_offset, ppa_ack->dl_length);
	ppa_count = ppa_ack->dl_count;

	for (count = 0, ppa_info = (dl_hp_ppa_info_t*) ppa_area;
	     count < ppa_count;
	     count++) {
		dl_get_statistics_req_t *get_statistics_req = (dl_get_statistics_req_t*) ctrl_area;
		dl_get_statistics_ack_t *get_statistics_ack = (dl_get_statistics_ack_t*) ctrl_area;

		dl_attach_req_t *attach_req;
		dl_detach_req_t *detach_req;
		mib_ifEntry *mib_ptr;
#ifdef HP_UX11
		mib_Dot3StatsEntry *mib_Dot3_ptr;
#endif

		attach_req = (dl_attach_req_t*) ctrl_area;
		attach_req->dl_primitive = DL_ATTACH_REQ;
		attach_req->dl_ppa = ppa_info[count].dl_ppa;
		ctrl_buf.len = sizeof(dl_attach_req_t);
		if (putmsg(fd, &ctrl_buf, 0, 0) < 0) {
			sg_set_error_with_errno(SG_ERROR_PUTMSG, "DL_ATTACH_REQ");
			return NULL;
		}

		ctrl_area[0] = 0;
		if (getmsg(fd, &ctrl_buf, &data_buf, &flags) < 0) {
			sg_set_error_with_errno(SG_ERROR_GETMSG, "DL_ATTACH_REQ");
			return NULL;
		}

		get_statistics_req->dl_primitive = DL_GET_STATISTICS_REQ;
		ctrl_buf.len = sizeof(dl_get_statistics_req_t);

		if (putmsg(fd, &ctrl_buf, NULL, 0) < 0) {
			sg_set_error_with_errno(SG_ERROR_PUTMSG, "DL_GET_STATISTICS_REQ");
			return NULL;
		}

		flags = 0;
		ctrl_area[0] = 0;

		if (getmsg(fd, &ctrl_buf, NULL, &flags) < 0) {
			sg_set_error_with_errno(SG_ERROR_GETMSG, "DL_GET_STATISTICS_REQ");
			return NULL;
		}
		if (get_statistics_ack->dl_primitive != DL_GET_STATISTICS_ACK) {
			sg_set_error_with_errno(SG_ERROR_PARSE, "DL_GET_STATISTICS_ACK");
			return NULL;
		}

		mib_ptr = (mib_ifEntry*) ((u_char*) ctrl_area + get_statistics_ack->dl_stat_offset);

		if (0 == (mib_ptr->ifOper & 1)) {
			continue;
		}

		if (VECTOR_RESIZE(network_stats, interfaces + 1) < 0) {
			return NULL;
		}
		network_stat_ptr=network_stats+interfaces;

		snprintf( name_buf, sizeof(name_buf), "lan%d", ppa_info[count].dl_ppa );

		if (sg_update_string(&network_stat_ptr->interface_name, name_buf ) < 0 ) {
			return NULL;
		}
		network_stat_ptr->rx = mib_ptr->ifInOctets;
		network_stat_ptr->tx = mib_ptr->ifOutOctets;
		network_stat_ptr->ipackets = mib_ptr->ifInUcastPkts + mib_ptr->ifInNUcastPkts;
		network_stat_ptr->opackets = mib_ptr->ifOutUcastPkts + mib_ptr->ifOutNUcastPkts;
		network_stat_ptr->ierrors = mib_ptr->ifInErrors;
		network_stat_ptr->oerrors = mib_ptr->ifOutErrors;
#ifdef HP_UX11
		mib_Dot3_ptr = (mib_Dot3StatsEntry *) (void *) ((int) mib_ptr + sizeof (mib_ifEntry));
		network_stat_ptr->collisions = mib_Dot3_ptr->dot3StatsSingleCollisionFrames
		                             + mib_Dot3_ptr->dot3StatsMultipleCollisionFrames;
#else
		network_stat_ptr->collisions = 0; /* currently unknown */
#endif
		network_stat_ptr->systime = time(NULL);
		interfaces++;

		detach_req = (dl_detach_req_t*) ctrl_area;
		detach_req->dl_primitive = DL_DETACH_REQ;
		ctrl_buf.len = sizeof(dl_detach_req_t);
		if (putmsg(fd, &ctrl_buf, 0, 0) < 0) {
			sg_set_error_with_errno(SG_ERROR_PUTMSG, "DL_DETACH_REQ");
			return NULL;
		}

		ctrl_area[0] = 0;
		flags = 0;
		if (getmsg(fd, &ctrl_buf, &data_buf, &flags) < 0) {
			sg_set_error_with_errno(SG_ERROR_GETMSG, "DL_DETACH_REQ");
			return NULL;
		}
	}

	close(fd);

	free(ctrl_area);
	free(data_area);
	free(ppa_area);

#endif

#ifdef ALLBSD
	if (getifaddrs(&net) != 0){
		sg_set_error_with_errno(SG_ERROR_GETIFADDRS, NULL);
		return NULL;
	}

	interfaces=0;
	
	for(net_ptr=net;net_ptr!=NULL;net_ptr=net_ptr->ifa_next){
		if (net_ptr->ifa_addr->sa_family != AF_LINK) continue;

		if (VECTOR_RESIZE(network_stats, interfaces + 1) < 0) {
			return NULL;
		}
		network_stat_ptr=network_stats+interfaces;
		
		if (sg_update_string(&network_stat_ptr->interface_name,
				     net_ptr->ifa_name) < 0) {
			return NULL;
		}
		net_data=(struct if_data *)net_ptr->ifa_data;
		network_stat_ptr->rx=net_data->ifi_ibytes;
		network_stat_ptr->tx=net_data->ifi_obytes;
		network_stat_ptr->ipackets=net_data->ifi_ipackets;
		network_stat_ptr->opackets=net_data->ifi_opackets;
		network_stat_ptr->ierrors=net_data->ifi_ierrors;
		network_stat_ptr->oerrors=net_data->ifi_oerrors;
		network_stat_ptr->collisions=net_data->ifi_collisions;
		network_stat_ptr->systime=time(NULL);
		interfaces++;
	}
	freeifaddrs(net);	
#endif
#ifdef AIX
	/* check how many perfstat_netinterface_t structures are available */
	interfaces = perfstat_netinterface(NULL, NULL, sizeof(perfstat_netinterface_t), 0);

	/* allocate enough memory for all the structures */
	statp = calloc(interfaces, sizeof(perfstat_netinterface_t));

	/* set name to first interface */
	strcpy(first.name, FIRST_NETINTERFACE);

	/* ask to get all the structures available in one call */
	/* return code is number of structures returned */
	interfaces = perfstat_netinterface(&first, statp, sizeof(perfstat_netinterface_t), interfaces);
	if (-1 == interfaces) {
		sg_set_error(SG_ERROR_SYSCTLBYNAME, "perfstat_netinterface");
		free(statp);
		return NULL;
	}

	if (VECTOR_RESIZE(network_stats, interfaces) < 0) {
		sg_set_error(SG_ERROR_MALLOC, NULL);
		free(statp);
		return NULL;
	}

	/* print statistics for each of the interfaces */
	for (i = 0; i < interfaces; i++) {
		if (sg_update_string(&network_stats[i].interface_name, statp[i].name) < 0) {
			free(statp);
			return NULL;
		}
		network_stats[i].tx = statp[i].obytes;
		network_stats[i].rx = statp[i].ibytes;
		network_stats[i].opackets = statp[i].opackets;
		network_stats[i].ipackets = statp[i].ipackets;
		network_stats[i].oerrors = statp[i].oerrors;
		network_stats[i].ierrors = statp[i].ierrors;
		network_stats[i].collisions = statp[i].collisions;
		network_stats[i].systime = time(NULL);
	}

	free(statp);
#endif
#ifdef SOLARIS
	if ((kc = kstat_open()) == NULL) {
		sg_set_error(SG_ERROR_KSTAT_OPEN, NULL);
		return NULL;
	}

	interfaces=0;

	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
		if (strcmp(ksp->ks_class, "net") == 0) {
			kstat_read(kc, ksp, NULL);

#ifdef SOL7
#define LRX "rbytes"
#define LTX "obytes"
#define LIPACKETS "ipackets"
#define LOPACKETS "opackets"
#define VALTYPE value.ui32
#else
#define LRX "rbytes64"
#define LTX "obytes64"
#define LIPACKETS "ipackets64"
#define LOPACKETS "opackets64"
#define VALTYPE value.ui64
#endif

			/* Read rx */
			if((knp=kstat_data_lookup(ksp, LRX))==NULL){
				/* This is a network interface, but it doesn't
				 * have the rbytes/obytes values; for instance,
				 * the loopback devices have this behaviour
				 * (although they do track packets in/out). */
				/* FIXME: Show packet counts when byte counts
				 * not available. */
				continue;
			}

			/* Create new network_stats */
			if (VECTOR_RESIZE(network_stats, interfaces + 1) < 0) {
				kstat_close(kc);
				return NULL;
			}
			network_stat_ptr=network_stats+interfaces;

			/* Read interface name */
			if (sg_update_string(&network_stat_ptr->interface_name,
					     ksp->ks_name) < 0) {
				kstat_close(kc);
				return NULL;
			}

			/* Finish reading rx */
			network_stat_ptr->rx=knp->VALTYPE;

			/* Read tx */
			if((knp=kstat_data_lookup(ksp, LTX))==NULL){
				continue;
			}
			network_stat_ptr->tx=knp->VALTYPE;

			/* Read ipackets */
			if((knp=kstat_data_lookup(ksp, LIPACKETS))==NULL){
				continue;
			}
			network_stat_ptr->ipackets=knp->VALTYPE;

			/* Read opackets */
			if((knp=kstat_data_lookup(ksp, LOPACKETS))==NULL){
				continue;
			}
			network_stat_ptr->opackets=knp->VALTYPE;

			/* Read ierrors */
			if((knp=kstat_data_lookup(ksp, "ierrors"))==NULL){
				continue;
			}
			network_stat_ptr->ierrors=knp->value.ui32;

			/* Read oerrors */
			if((knp=kstat_data_lookup(ksp, "oerrors"))==NULL){
				continue;
			}
			network_stat_ptr->oerrors=knp->value.ui32;

			/* Read collisions */
			if((knp=kstat_data_lookup(ksp, "collisions"))==NULL){
				continue;
			}
			network_stat_ptr->collisions=knp->value.ui32;


			/* Store systime */
			network_stat_ptr->systime=time(NULL);

			interfaces++;
		}
	}
		
	kstat_close(kc);	
#endif
#ifdef LINUX
	f=fopen("/proc/net/dev", "r");
	if(f==NULL){
		sg_set_error_with_errno(SG_ERROR_OPEN, "/proc/net/dev");
		return NULL;
	}
	/* read the 2 lines.. Its the title, so we dont care :) */
	fgets(line, sizeof(line), f);
	fgets(line, sizeof(line), f);


	if((regcomp(&regex, "^ *([^:]+): *([0-9]+) +([0-9]+) +([0-9]+) +[0-9]+ +[0-9]+ +[0-9]+ +[0-9]+ +[0-9]+ +([0-9]+) +([0-9]+) +([0-9]+) +[0-9]+ +[0-9]+ +([0-9]+)", REG_EXTENDED))!=0){
		sg_set_error(SG_ERROR_PARSE, NULL);
		return NULL;
	}

	interfaces=0;

	while((fgets(line, sizeof(line), f)) != NULL){
		if((regexec(&regex, line, 9, line_match, 0))!=0){
			continue;
		}

		if (VECTOR_RESIZE(network_stats, interfaces + 1) < 0) {
			return NULL;
		}
		network_stat_ptr=network_stats+interfaces;

		if(network_stat_ptr->interface_name!=NULL){
			free(network_stat_ptr->interface_name);
		}

		network_stat_ptr->interface_name=sg_get_string_match(line, &line_match[1]);
		network_stat_ptr->rx=sg_get_ll_match(line, &line_match[2]);
		network_stat_ptr->tx=sg_get_ll_match(line, &line_match[5]);
		network_stat_ptr->ipackets=sg_get_ll_match(line, &line_match[3]);
		network_stat_ptr->opackets=sg_get_ll_match(line, &line_match[6]);
		network_stat_ptr->ierrors=sg_get_ll_match(line, &line_match[4]);
		network_stat_ptr->oerrors=sg_get_ll_match(line, &line_match[7]);
		network_stat_ptr->collisions=sg_get_ll_match(line, &line_match[8]);
		network_stat_ptr->systime=time(NULL);

		interfaces++;
	}
	fclose(f);
	regfree(&regex);

#endif

#ifdef CYGWIN
	sg_set_error(SG_ERROR_UNSUPPORTED, "Cygwin");
	return NULL;
#endif

#ifdef WIN32
	interfaces = 0;

	if((if_table = win32_get_devices()) == NULL) {
		sg_set_error(SG_ERROR_DEVICES, "network");
		return NULL;
	}

	if(VECTOR_RESIZE(network_stats, if_table->dwNumEntries) < 0) {
		free(if_table);
		return NULL;
	}

	for (i=0; i<if_table->dwNumEntries; i++) {
		network_stat_ptr=network_stats+i;
		if_row = if_table->table[i];

		if(sg_update_string(&network_stat_ptr->interface_name,
					if_row.bDescr) < 0) {
			free(if_table);
			return NULL;
		}
		network_stat_ptr->tx = if_row.dwOutOctets;
		network_stat_ptr->rx = if_row.dwInOctets;
		network_stat_ptr->ipackets = if_row.dwInUcastPkts + if_row.dwInNUcastPkts;
		network_stat_ptr->opackets = if_row.dwOutUcastPkts + if_row.dwOutNUcastPkts;
		network_stat_ptr->ierrors = if_row.dwInErrors;
		network_stat_ptr->oerrors = if_row.dwOutErrors;
		network_stat_ptr->collisions = 0; /* can't do that */
		network_stat_ptr->systime = time(NULL);

		interfaces++;
	}
	free(if_table);

	/* Please say there's a nicer way to do this...  If windows has two (or
	 * more) identical network cards, GetIfTable returns them with the same
	 * name, not like in Device Manager where the other has a #2 etc after
	 * it. So, add the #number here. Should we be doing this? Or should the
	 * end programs be dealing with duplicate names? Currently breaks
	 * watch.pl in rrdgraphing. But Unix does not have the issue of
	 * duplicate net device names.
	 */
	for (i=0; i<interfaces; i++) {
		no = 2;
		for(j=i+1; j<interfaces; j++) {
			network_stat_ptr=network_stats+j;
			if(strcmp(network_stats[i].interface_name, 
					network_stat_ptr->interface_name) == 0) {
				if(snprintf(buf, sizeof(buf), " #%d", no) < 0) {
					break;
				}
				if(sg_concat_string(&network_stat_ptr->interface_name, buf) != 0) {
					return NULL;
				}

				no++;
			}
		}
	}
#endif

	*entries=interfaces;

	return network_stats;	
}

static long long transfer_diff(long long new, long long old){
#if defined(SOL7) || defined(LINUX) || defined(FREEBSD) || defined(DFBSD) || defined(OPENBSD) || defined(WIN32)
	/* 32-bit quantities, so we must explicitly deal with wraparound. */
#define MAXVAL 0x100000000LL
	if (new >= old) {
		return new - old;
	} else {
		return MAXVAL + new - old;
	}
#else
	/* 64-bit quantities, so plain subtraction works. */
	return new - old;
#endif
}

sg_network_io_stats *sg_get_network_io_stats_diff(int *entries) {
	VECTOR_DECLARE_STATIC(diff, sg_network_io_stats, 1,
			      network_stat_init, network_stat_destroy);
	sg_network_io_stats *src = NULL, *dest;
	int i, j, diff_count, new_count;

	if (network_stats == NULL) {
		/* No previous stats, so we can't calculate a difference. */
		return sg_get_network_io_stats(entries);
	}

	/* Resize the results array to match the previous stats. */
	diff_count = VECTOR_SIZE(network_stats);
	if (VECTOR_RESIZE(diff, diff_count) < 0) {
		return NULL;
	}

	/* Copy the previous stats into the result. */
	for (i = 0; i < diff_count; i++) {
		src = &network_stats[i];
		dest = &diff[i];

		if (sg_update_string(&dest->interface_name,
				     src->interface_name) < 0) {
			return NULL;
		}
		dest->rx = src->rx;
		dest->tx = src->tx;
		dest->ipackets = src->ipackets;
		dest->opackets = src->opackets;
		dest->ierrors = src->ierrors;
		dest->oerrors = src->oerrors;
		dest->collisions = src->collisions;
		dest->systime = src->systime;
	}

	/* Get a new set of stats. */
	if (sg_get_network_io_stats(&new_count) == NULL) {
		return NULL;
	}

	/* For each previous stat... */
	for (i = 0; i < diff_count; i++) {
		dest = &diff[i];

		/* ... find the corresponding new stat ... */
		for (j = 0; j < new_count; j++) {
			/* Try the new stat in the same position first,
			   since that's most likely to be it. */
			src = &network_stats[(i + j) % new_count];
			if (strcmp(src->interface_name, dest->interface_name) == 0) {
				break;
			}
		}
		if (j == new_count) {
			/* No match found. */
			continue;
		}

		/* ... and subtract the previous stat from it to get the
		   difference. */
		dest->rx = transfer_diff(src->rx, dest->rx);
		dest->tx = transfer_diff(src->tx, dest->tx);
		dest->ipackets = transfer_diff(src->ipackets, dest->ipackets);
		dest->opackets = transfer_diff(src->opackets, dest->opackets);
		dest->ierrors = transfer_diff(src->ierrors, dest->ierrors);
		dest->oerrors = transfer_diff(src->oerrors, dest->oerrors);
		dest->collisions = transfer_diff(src->collisions, dest->collisions);
		dest->systime = src->systime - dest->systime;
	}

	*entries = diff_count;
	return diff;
}

int sg_network_io_compare_name(const void *va, const void *vb) {
	const sg_network_io_stats *a = (const sg_network_io_stats *)va;
	const sg_network_io_stats *b = (const sg_network_io_stats *)vb;

	return strcmp(a->interface_name, b->interface_name);
}

/* NETWORK INTERFACE STATS */

static void network_iface_stat_init(sg_network_iface_stats *s) {
	s->interface_name = NULL;
	s->speed = 0;
	s->duplex = SG_IFACE_DUPLEX_UNKNOWN;
}

static void network_iface_stat_destroy(sg_network_iface_stats *s) {
	free(s->interface_name);
}

sg_network_iface_stats *sg_get_network_iface_stats(int *entries){
	VECTOR_DECLARE_STATIC(network_iface_stats, sg_network_iface_stats, 5,
			      network_iface_stat_init, network_iface_stat_destroy);
	sg_network_iface_stats *network_iface_stat_ptr;
	int ifaces = 0;

#ifdef SOLARIS
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_named_t *knp;
	int sock;
#endif
#ifdef ALLBSD
	struct ifmediareq ifmed;
	struct ifaddrs *net, *net_ptr;
	struct ifreq ifr;
	int sock;
	int x;
#endif
#ifdef LINUX
	FILE *f;
	/* Horrible big enough, but it should be easily big enough */
	char line[8096];
	int sock;
#endif
#ifdef WIN32
	PMIB_IFTABLE if_table;
	MIB_IFROW if_row;
	int i,j,no;
	char buf[5];
#endif
#ifdef AIX
	int fd, n;
	size_t pagesize;
	struct ifreq *ifr, *lifr, iff;
	struct ifconf ifc;
#endif
#ifdef HPUX
	int fd;
	struct ifreq *ifr, *lifr, iff;
	struct ifconf ifc;
#endif

#ifdef ALLBSD
	if(getifaddrs(&net) != 0){
		sg_set_error_with_errno(SG_ERROR_GETIFADDRS, NULL);
		return NULL;
	}

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == 0) return NULL;

	for(net_ptr=net; net_ptr!=NULL; net_ptr=net_ptr->ifa_next){
		if(net_ptr->ifa_addr->sa_family != AF_LINK) continue;

		if (VECTOR_RESIZE(network_iface_stats, ifaces + 1) < 0) {
			return NULL;
		}
		network_iface_stat_ptr = network_iface_stats + ifaces;

		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, net_ptr->ifa_name, sizeof(ifr.ifr_name));

		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0){
			continue;
		}	
		if((ifr.ifr_flags & IFF_UP) != 0){
			network_iface_stat_ptr->up = 1;
		}else{
			network_iface_stat_ptr->up = 0;
		}

		if (sg_update_string(&network_iface_stat_ptr->interface_name,
				     net_ptr->ifa_name) < 0) {
			return NULL;
		}

		network_iface_stat_ptr->speed = 0;
		network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_UNKNOWN;
		ifaces++;

		memset(&ifmed, 0, sizeof(struct ifmediareq));
		sg_strlcpy(ifmed.ifm_name, net_ptr->ifa_name, sizeof(ifmed.ifm_name));
		if(ioctl(sock, SIOCGIFMEDIA, (caddr_t)&ifmed) == -1){
			/* Not all interfaces support the media ioctls. */
			continue;
		}

		/* We may need to change this if we start doing wireless devices too */
		if( (ifmed.ifm_active | IFM_ETHER) != ifmed.ifm_active ){
			/* Not a ETHER device */
			continue;
		}

		/* Assuming only ETHER devices */
		x = IFM_SUBTYPE(ifmed.ifm_active);
		switch(x){
			/* 10 Mbit connections. Speedy :) */
			case(IFM_10_T):
			case(IFM_10_2):
			case(IFM_10_5):
			case(IFM_10_STP):
			case(IFM_10_FL):
				network_iface_stat_ptr->speed = 10;
				break;
			/* 100 Mbit connections */
			case(IFM_100_TX):
			case(IFM_100_FX):
			case(IFM_100_T4):
			case(IFM_100_VG):
			case(IFM_100_T2):
				network_iface_stat_ptr->speed = 100;
				break;
			/* 1000 Mbit connections */
			case(IFM_1000_SX):
			case(IFM_1000_LX):
			case(IFM_1000_CX):
#if defined(IFM_1000_TX) && !defined(OPENBSD)
			case(IFM_1000_TX): /* FreeBSD 4 and others (but NOT OpenBSD)? */
#endif
#ifdef IFM_1000_FX
			case(IFM_1000_FX): /* FreeBSD 4 */
#endif
#ifdef IFM_1000_T
			case(IFM_1000_T): /* FreeBSD 5 */
#endif
				network_iface_stat_ptr->speed = 1000;
				break;
			/* We don't know what it is */
			default:
				network_iface_stat_ptr->speed = 0;
				break;
		}

		if( (ifmed.ifm_active | IFM_FDX) == ifmed.ifm_active ){
			network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_FULL;
		}else if( (ifmed.ifm_active | IFM_HDX) == ifmed.ifm_active ){
			network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_HALF;
		}else{
			network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_UNKNOWN;
		}
	}	
	freeifaddrs(net);
	close(sock);
#endif

#ifdef AIX
/*
 * Fix up variable length of struct ifr
 */
#ifndef _SIZEOF_ADDR_IFREQ
#define _SIZEOF_ADDR_IFREQ(ifr) \
	(((ifr).ifr_addr.sa_len > sizeof(struct sockaddr)) \
	? sizeof(struct ifreq) - sizeof(struct sockaddr) + (ifr).ifr_addr.sa_len \
	: sizeof(struct ifreq))
#endif

	if ((pagesize = sysconf(_SC_PAGESIZE)) == ((size_t)-1)) {
		sg_set_error_with_errno(SG_ERROR_SYSCONF, "_SC_PAGESIZE");
		return NULL;
	}

	if ((fd = socket (PF_INET, SOCK_DGRAM, 0)) == -1) {
		return NULL;
	}

	n = 2;
	ifr = (struct ifreq *)malloc( n*pagesize );
	if( NULL == ifr ) {
		sg_set_error_with_errno(SG_ERROR_MALLOC, NULL);
		close (fd);
		return NULL;
	}
	bzero(&ifc, sizeof(ifc));
	ifc.ifc_req = ifr;
	ifc.ifc_len = n * pagesize;
	while( ( ioctl( fd, SIOCGIFCONF, &ifc ) == -1 ) ||
	       ( ((size_t)ifc.ifc_len) >= ( (n-1) * pagesize)) )
	{
		n *= 2;
		ifr = (struct ifreq *)realloc( ifr, n*pagesize );
		if( NULL == ifr ) {
			free( ifc.ifc_req );
			close (fd);
			sg_set_error_with_errno(SG_ERROR_MALLOC, NULL);
			return NULL;
		}
		ifc.ifc_req = ifr;
		ifc.ifc_len = n * pagesize;
	}

	lifr = (struct ifreq *)&ifc.ifc_buf[ifc.ifc_len];
	for ( ifr = ifc.ifc_req;
	      ifr < lifr;
	      ifr = (struct ifreq *)&(((char *)ifr)[_SIZEOF_ADDR_IFREQ(*ifr)]) )
	{
		struct sockaddr *sa = &ifr->ifr_addr;

		if(sa->sa_family != AF_LINK)
			continue;

		if (VECTOR_RESIZE(network_iface_stats, ifaces + 1) < 0) {
			free( ifc.ifc_req );
			close (fd);
			return NULL;
		}
		network_iface_stat_ptr = network_iface_stats + ifaces;

		memset(&iff, 0, sizeof(iff));
		strncpy(iff.ifr_name, ifr->ifr_name, sizeof(iff.ifr_name));

		if (ioctl(fd, SIOCGIFFLAGS, &iff) < 0) {
			continue;
		}

		if ((iff.ifr_flags & IFF_UP) != 0) {
			network_iface_stat_ptr->up = 1;
		} else {
			network_iface_stat_ptr->up = 0;
		}
		if (sg_update_string(&network_iface_stat_ptr->interface_name,
				     ifr->ifr_name) < 0) {
			close (fd);
			free( ifc.ifc_req );
			return NULL;
		}

		network_iface_stat_ptr->speed = 0;
		network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_UNKNOWN;
		ifaces++;
	}

	close (fd);
	free( ifc.ifc_req );
#endif

#ifdef HPUX
/*
 * Fix up variable length of struct ifr
 */
#ifndef _SA_LEN
#define _SA_LEN(sa) \
   ((sa).sa_family == AF_UNIX ? sizeof(struct sockaddr_un) : \
   (sa).sa_family == AF_INET6 ? sizeof(struct sockaddr_in6) : \
   sizeof(struct sockaddr))
#endif

#ifndef _SIZEOF_ADDR_IFREQ
#define _SIZEOF_ADDR_IFREQ(ifr) \
	((_SA_LEN((ifr).ifr_addr) > sizeof(struct sockaddr)) \
	? sizeof(struct ifreq) - sizeof(struct sockaddr) + _SA_LEN((ifr).ifr_addr) \
	: sizeof(struct ifreq))
#endif
	if ((fd = socket (PF_INET, SOCK_DGRAM, 0)) == -1) {
		return NULL;
	}

	bzero(&ifc, sizeof(ifc));
	if (ioctl (fd, SIOCGIFNUM, &ifc) != -1) {
		ifr = calloc(sizeof (*ifr), ifc.ifc_len);
		if( NULL == ifr ) {
			close (fd);
			sg_set_error_with_errno(SG_ERROR_MALLOC, NULL);
			return NULL;
		}
		ifc.ifc_req = ifr;
		ifc.ifc_len *= sizeof (*ifr);
		if (ioctl (fd, SIOCGIFCONF, &ifc) == -1) {
			free (ifr);
			close (fd);
			sg_set_error_with_errno(SG_ERROR_GETIFADDRS, NULL);
			return NULL;
		}
	} else {
		close (fd);
		sg_set_error_with_errno(SG_ERROR_GETIFADDRS, NULL);
		return NULL;
	}

	lifr = (struct ifreq *)&ifc.ifc_buf[ifc.ifc_len];
	for ( ifr = ifc.ifc_req;
	      ifr < lifr;
	      ifr = (struct ifreq *)(((char *)ifr) + _SIZEOF_ADDR_IFREQ(*ifr)) )
	{
		if (VECTOR_RESIZE(network_iface_stats, ifaces + 1) < 0) {
			free( ifc.ifc_req );
			close (fd);
			return NULL;
		}
		network_iface_stat_ptr = network_iface_stats + ifaces;

		bzero(&iff, sizeof(iff));
		strncpy(iff.ifr_name, ifr->ifr_name, sizeof(iff.ifr_name));

		if (ioctl(fd, SIOCGIFFLAGS, &iff) < 0) {
			continue;
		}

		if ((iff.ifr_flags & IFF_UP) != 0) {
			network_iface_stat_ptr->up = 1;
		} else {
			network_iface_stat_ptr->up = 0;
		}
		if (sg_update_string(&network_iface_stat_ptr->interface_name,
				     ifr->ifr_name) < 0) {
			close (fd);
			free( ifc.ifc_req );
			return NULL;
		}

		/**
		 * for physical devices we now could ask /dev/dlpi for speed and type,
		 * but for monitoring we don't care
		 */
		network_iface_stat_ptr->speed = 0;
		network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_UNKNOWN;
		ifaces++;
	}

	close (fd);
	free( ifc.ifc_req );
#endif

#ifdef SOLARIS
	if ((kc = kstat_open()) == NULL) {
		sg_set_error(SG_ERROR_KSTAT_OPEN, NULL);
		return NULL;
	}

	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) {
		sg_set_error_with_errno(SG_ERROR_SOCKET, NULL);
		kstat_close(kc);
		return NULL;
	}

	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
		if (strcmp(ksp->ks_class, "net") == 0) {
			struct ifreq ifr;

			kstat_read(kc, ksp, NULL);

			strncpy(ifr.ifr_name, ksp->ks_name, sizeof ifr.ifr_name);
			if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
				/* Not a network interface. */
				continue;
			}

			if (VECTOR_RESIZE(network_iface_stats, ifaces + 1) < 0) {
				kstat_close(kc);
				return NULL;
			}
			network_iface_stat_ptr = network_iface_stats + ifaces;
			ifaces++;

			if (sg_update_string(&network_iface_stat_ptr->interface_name,
					     ksp->ks_name) < 0) {
				kstat_close(kc);
				return NULL;
			}

			if ((ifr.ifr_flags & IFF_UP) != 0) {
				if ((knp = kstat_data_lookup(ksp, "link_up")) != NULL) {
					/* take in to account if link
					 * is up as well as interface */
					if (knp->value.ui32 != 0u) {
						network_iface_stat_ptr->up = 1;
					} else {
						network_iface_stat_ptr->up = 0;
					}
				}
				else {
					/* maintain compatibility */
					network_iface_stat_ptr->up = 1;
				}
			} else {
				network_iface_stat_ptr->up = 0;
			}

			if ((knp = kstat_data_lookup(ksp, "ifspeed")) != NULL) {
				network_iface_stat_ptr->speed = knp->value.ui64 / (1000 * 1000);
			} else {
				network_iface_stat_ptr->speed = 0;
			}

			network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_UNKNOWN;
			if ((knp = kstat_data_lookup(ksp, "link_duplex")) != NULL) {
				switch (knp->value.ui32) {
				case 1:
					network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_HALF;
					break;
				case 2:
					network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_FULL;
					break;
				}
			}
		}
	}

	close(sock);
	kstat_close(kc);
#endif
#ifdef LINUX
	f = fopen("/proc/net/dev", "r");
	if(f == NULL){
		sg_set_error_with_errno(SG_ERROR_OPEN, "/proc/net/dev");
		return NULL;
	}

	/* Setup stuff so we can do the ioctl to get the info */
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		sg_set_error_with_errno(SG_ERROR_SOCKET, NULL);
		return NULL;
	}

	/* Ignore first 2 lines.. Just headings */
	if((fgets(line, sizeof(line), f)) == NULL) {
		sg_set_error(SG_ERROR_PARSE, NULL);
		return NULL;
	}
	if((fgets(line, sizeof(line), f)) == NULL) {
		sg_set_error(SG_ERROR_PARSE, NULL);
		return NULL;
	}

	while((fgets(line, sizeof(line), f)) != NULL){
		char *name, *ptr;
		struct ifreq ifr;
		struct ethtool_cmd ethcmd;
		int err;

		/* Get the interface name */
		ptr = strchr(line, ':');
		if (ptr == NULL) continue;
		*ptr='\0';
		name = line;
		while(isspace(*(name))){
			name++;
		}

		memset(&ifr, 0, sizeof ifr);
		strncpy(ifr.ifr_name, name, sizeof ifr.ifr_name);

		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
			continue;
		}

		/* We have a good interface to add */
		if (VECTOR_RESIZE(network_iface_stats, ifaces + 1) < 0) {
			return NULL;
		}
		network_iface_stat_ptr = network_iface_stats + ifaces;
		
		if (sg_update_string(&network_iface_stat_ptr->interface_name,
				     name) < 0) {
			return NULL;
		}
		if ((ifr.ifr_flags & IFF_UP) != 0) {
			network_iface_stat_ptr->up = 1;
		} else {
			network_iface_stat_ptr->up = 0;
		}

		memset(&ethcmd, 0, sizeof ethcmd);
		ethcmd.cmd = ETHTOOL_GSET;
		ifr.ifr_data = (caddr_t) &ethcmd;

		err = ioctl(sock, SIOCETHTOOL, &ifr);
		if (err == 0) {
			network_iface_stat_ptr->speed = ethcmd.speed;

			switch (ethcmd.duplex) {
			case DUPLEX_FULL:
				network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_FULL;
				break;
			case DUPLEX_HALF:
				network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_HALF;
				break;
			default:
				network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_UNKNOWN;
			}
		} else {
			/* Not all interfaces support the ethtool ioctl. */
			network_iface_stat_ptr->speed = 0;
			network_iface_stat_ptr->duplex = SG_IFACE_DUPLEX_UNKNOWN;
		}

		ifaces++;
	}
	close(sock);
	fclose(f);
#endif
#ifdef CYGWIN
	sg_set_error(SG_ERROR_UNSUPPORTED, "Cygwin");
	return NULL;
#endif
#ifdef WIN32
	ifaces = 0;

	if((if_table = win32_get_devices()) == NULL) {
		sg_set_error(SG_ERROR_DEVICES, "network interfaces");
		return NULL;
	}

	if(VECTOR_RESIZE(network_iface_stats, if_table->dwNumEntries) < 0) {
		free(if_table);
		return NULL;
	}

	for(i=0; i<if_table->dwNumEntries; i++) {
		network_iface_stat_ptr=network_iface_stats+i;
		if_row = if_table->table[i];

		if(sg_update_string(&network_iface_stat_ptr->interface_name,
					if_row.bDescr) < 0) {
			free(if_table);
			return NULL;
		}
		network_iface_stat_ptr->speed = if_row.dwSpeed /1000000;

		if((if_row.dwOperStatus == MIB_IF_OPER_STATUS_CONNECTED ||
				if_row.dwOperStatus == 
					MIB_IF_OPER_STATUS_OPERATIONAL) &&
				if_row.dwAdminStatus == 1) {
			network_iface_stat_ptr->up = 1;
		} else {
			network_iface_stat_ptr->up = 0;
		}

		ifaces++;
	}
	free(if_table);

	/* again with the renumbering */
	for (i=0; i<ifaces; i++) {
		no = 2;
		for(j=i+1; j<ifaces; j++) {
			network_iface_stat_ptr=network_iface_stats+j;
			if(strcmp(network_iface_stats[i].interface_name, 
					network_iface_stat_ptr->interface_name) == 0) {
				if(snprintf(buf, sizeof(buf), " #%d", no) < 0) {
					break;
				}
				if(sg_concat_string(&network_iface_stat_ptr->interface_name, buf) != 0) {
					return NULL;
				}

				no++;
			}
		}
	}
#endif

#ifdef SG_ENABLE_DEPRECATED
	network_iface_stat_ptr->dup = network_iface_stat_ptr->duplex;
#endif

	*entries = ifaces;
	return network_iface_stats; 
}

int sg_network_iface_compare_name(const void *va, const void *vb) {
	const sg_network_iface_stats *a = (const sg_network_iface_stats *)va;
	const sg_network_iface_stats *b = (const sg_network_iface_stats *)vb;

	return strcmp(a->interface_name, b->interface_name);
}

