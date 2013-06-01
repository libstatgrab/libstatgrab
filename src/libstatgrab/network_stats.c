/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2013 i-scream
 * Copyright (C) 2010-2013 Jens Rehsack
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

#include "tools.h"

static void
sg_network_io_stats_item_init(sg_network_io_stats *d) {
	memset( d, 0, sizeof(*d) );
}

static sg_error
sg_network_io_stats_item_copy(sg_network_io_stats *d, const sg_network_io_stats *s) {

	if( SG_ERROR_NONE != sg_update_string(&d->interface_name, s->interface_name) ) {
		RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
	}

	d->tx = s->tx;
	d->rx = s->rx;
	d->ipackets = s->ipackets;
	d->opackets = s->opackets;
	d->ierrors = s->ierrors;
	d->oerrors = s->oerrors;
	d->collisions = s->collisions;
	d->systime = s->systime;

	return SG_ERROR_NONE;
}

static long long
transfer_diff(long long new, long long old){
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

static sg_error
sg_network_io_stats_item_compute_diff(const sg_network_io_stats *s, sg_network_io_stats *d) {

	d->tx = transfer_diff( d->tx, s->tx );
	d->rx = transfer_diff( d->rx, s->rx );
	d->ipackets = transfer_diff( d->ipackets, s->ipackets );
	d->opackets = transfer_diff( d->opackets, s->opackets );
	d->ierrors = transfer_diff( d->ierrors, s->ierrors );
	d->oerrors = transfer_diff( d->oerrors, s->oerrors );
	d->collisions = transfer_diff( d->collisions, s->collisions );
	d->systime -= s->systime;

	return SG_ERROR_NONE;
}

static int
sg_network_io_stats_item_compare(const sg_network_io_stats *a, const sg_network_io_stats *b) {

	return strcmp( a->interface_name, b->interface_name );
}

static void
sg_network_io_stats_item_destroy(sg_network_io_stats *d) {
	free(d->interface_name);
}

static void
sg_network_iface_stats_item_init(sg_network_iface_stats *d) {
	d->interface_name = NULL;
	d->speed = 0;
	d->duplex = SG_IFACE_DUPLEX_UNKNOWN;
#ifdef SG_ENABLE_DEPRECATED
	d->dup = SG_IFACE_DUPLEX_UNKNOWN;
#endif
	d->up = 0;
}

static sg_error
sg_network_iface_stats_item_copy(sg_network_iface_stats *d, const sg_network_iface_stats *s) {

	if( SG_ERROR_NONE != sg_update_string(&d->interface_name, s->interface_name) ) {
		RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
	}

	d->speed = s->speed;
	d->duplex = s->duplex;
#ifdef SG_ENABLE_DEPRECATED
	d->dup = s->dup;
#endif
	d->up = s->up;

	return SG_ERROR_NONE;
}

#define sg_network_iface_stats_item_compute_diff NULL
#define sg_network_iface_stats_item_compare NULL

static void
sg_network_iface_stats_item_destroy(sg_network_iface_stats *d) {
	free(d->interface_name);
}

VECTOR_INIT_INFO_FULL_INIT(sg_network_io_stats);
VECTOR_INIT_INFO_FULL_INIT(sg_network_iface_stats);

/*
 * setup code
 */

#define SG_NETWORK_IO_NOW_IDX	0
#define SG_NETWORK_IO_DIFF_IDX	1
#define SG_NETWORK_IFACE_IDX	2
#define SG_NETWORK_MAX_IDX	3

EXTENDED_COMP_SETUP(network,SG_NETWORK_MAX_IDX,NULL);

#ifdef LINUX
static regex_t network_io_rx;
#define RX_MATCH_COUNT (8+1)
#endif

sg_error
sg_network_init_comp(unsigned id) {
	GLOBAL_SET_ID(network,id);

#ifdef LINUX
	if (regcomp(&network_io_rx, "^[[:space:]]*([^:]+):[[:space:]]*" \
				    "([0-9]+)[[:space:]]+" \
				    "([0-9]+)[[:space:]]+" \
				    "([0-9]+)[[:space:]]+" \
				    "[0-9]+[[:space:]]+" \
				    "[0-9]+[[:space:]]+" \
				    "[0-9]+[[:space:]]+" \
				    "[0-9]+[[:space:]]+" \
				    "[0-9]+[[:space:]]+" \
				    "([0-9]+)[[:space:]]+" \
				    "([0-9]+)[[:space:]]+" \
				    "([0-9]+)[[:space:]]+" \
				    "[0-9]+[[:space:]]+" \
				    "[0-9]+[[:space:]]+" \
				    "([0-9]+)", REG_EXTENDED)!=0) {
		RETURN_WITH_SET_ERROR("network", SG_ERROR_PARSE, "regcomp");
	}
#endif

	return SG_ERROR_NONE;
}

void
sg_network_destroy_comp(void) {
#ifdef LINUX
	regfree(&network_io_rx);
#endif
}

EASY_COMP_CLEANUP_FN(network,SG_NETWORK_MAX_IDX)

/* real stuff */

#ifdef WIN32
static PMIB_IFTABLE
win32_get_devices(void)
{
	PMIB_IFTABLE if_table;
	PMIB_IFTABLE tmp;
	unsigned long dwSize = 0;

	// Allocate memory for pointers
	if_table = sg_malloc(sizeof(MIB_IFTABLE));
	if( if_table == NULL ) {
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

static sg_error
sg_get_network_io_stats_int(sg_vector **network_io_stats_vector_ptr){
	ssize_t interfaces = 0;
	sg_network_io_stats *network_io_ptr;

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
#elif defined(SOLARIS)
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_named_t *knp;
#elif defined(LINUX)
	FILE *f;
	char line[1024];
	regmatch_t line_match[RX_MATCH_COUNT];
#elif defined(ALLBSD)
	struct ifaddrs *net, *net_ptr;
	struct if_data *net_data;
#elif defined(WIN32)
	PMIB_IFTABLE if_table;
	MIB_IFROW if_row;
	int i;

	/* used for duplicate interface names. 5 for space, hash, up to two
	 * numbers and terminating slash */
	char buf[5];
#elif defined(AIX)
	ssize_t i;
	perfstat_netinterface_t *statp;
	perfstat_id_t first;
#endif

#ifdef HPUX
	if ((NULL == ctrl_area) || (NULL == data_area) || (NULL == ppa_area)) {
		free(ctrl_area); free(data_area); free(ppa_area);
		RETURN_WITH_SET_ERROR("network", SG_ERROR_MALLOC, "sg_get_network_io_stats");
	}

	if ((fd = open("/dev/dlpi", O_RDWR)) < 0) {
		free(ctrl_area); free(data_area); free(ppa_area);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_OPEN, "/dev/dlpi");
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP close(fd); free(ctrl_area); free(data_area); free(ppa_area);

	ppa_req = (dl_hp_ppa_req_t *) ctrl_area;
	ppa_ack = (dl_hp_ppa_ack_t *) ctrl_area;

	ppa_req->dl_primitive = DL_HP_PPA_REQ;

	ctrl_buf.len = sizeof(dl_hp_ppa_req_t);
	if(putmsg(fd, &ctrl_buf, 0, 0) < 0) {
		VECTOR_UPDATE_ERROR_CLEANUP
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_PUTMSG, "DL_HP_PPA_REQ");
	}

	flags = 0;
	ctrl_area[0] = 0;

	if (getmsg(fd, &ctrl_buf, &data_buf, &flags) < 0) {
		VECTOR_UPDATE_ERROR_CLEANUP
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_GETMSG, "DL_HP_PPA_REQ");
	}

	if (ppa_ack->dl_length == 0) {
		VECTOR_UPDATE_ERROR_CLEANUP
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_PARSE, "DL_HP_PPA_REQ");
	}

	// save all the PPA information
	memcpy ((u_char*) ppa_area, (u_char*) ctrl_area + ppa_ack->dl_offset, ppa_ack->dl_length);
	ppa_count = ppa_ack->dl_count;

	for (count = 0, ppa_info = (dl_hp_ppa_info_t*) ppa_area;
	     count < ppa_count;
	     ++count) {
		dl_get_statistics_req_t *get_statistics_req = (dl_get_statistics_req_t*) ctrl_area;
		dl_get_statistics_ack_t *get_statistics_ack = (dl_get_statistics_ack_t*) ctrl_area;

		dl_attach_req_t *attach_req;
		dl_detach_req_t *detach_req;
		mib_ifEntry *mib_ptr;
#ifdef HPUX11
		mib_Dot3StatsEntry *mib_Dot3_ptr;
#endif

		attach_req = (dl_attach_req_t*) ctrl_area;
		attach_req->dl_primitive = DL_ATTACH_REQ;
		attach_req->dl_ppa = ppa_info[count].dl_ppa;
		ctrl_buf.len = sizeof(dl_attach_req_t);
		if (putmsg(fd, &ctrl_buf, 0, 0) < 0) {
			unsigned ppa_lan_number = ppa_info[count].dl_ppa;
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_PUTMSG, "DL_ATTACH_REQ ppa %u", ppa_lan_number);
		}

		ctrl_area[0] = 0;
		if (getmsg(fd, &ctrl_buf, &data_buf, &flags) < 0) {
			unsigned ppa_lan_number = ppa_info[count].dl_ppa;
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_GETMSG, "DL_ATTACH_REQ ppa %u", ppa_lan_number);
		}

		get_statistics_req->dl_primitive = DL_GET_STATISTICS_REQ;
		ctrl_buf.len = sizeof(dl_get_statistics_req_t);

		if (putmsg(fd, &ctrl_buf, NULL, 0) < 0) {
			unsigned ppa_lan_number = ppa_info[count].dl_ppa;
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_PUTMSG, "DL_GET_STATISTICS_REQ ppa %u", ppa_lan_number);
		}

		flags = 0;
		ctrl_area[0] = 0;

		if (getmsg(fd, &ctrl_buf, NULL, &flags) < 0) {
			unsigned ppa_lan_number = ppa_info[count].dl_ppa;
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_GETMSG, "DL_GET_STATISTICS_REQ ppa %u", ppa_lan_number);
		}
		if (get_statistics_ack->dl_primitive != DL_GET_STATISTICS_ACK) {
			unsigned ppa_lan_number = ppa_info[count].dl_ppa;
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_PARSE, "DL_GET_STATISTICS_ACK ppa %u", ppa_lan_number);
		}

		mib_ptr = (mib_ifEntry*) ((u_char*) ctrl_area + get_statistics_ack->dl_stat_offset);

		if (0 == (mib_ptr->ifOper & 1)) {
			continue;
		}

		VECTOR_UPDATE(network_io_stats_vector_ptr, interfaces + 1, network_io_ptr, sg_network_io_stats);

		snprintf( name_buf, sizeof(name_buf), "lan%d", ppa_info[count].dl_ppa );

		if ( SG_ERROR_NONE != sg_update_string(&network_io_ptr[interfaces].interface_name, name_buf ) ) {
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
		}
		network_io_ptr[interfaces].rx = mib_ptr->ifInOctets;
		network_io_ptr[interfaces].tx = mib_ptr->ifOutOctets;
		network_io_ptr[interfaces].ipackets = mib_ptr->ifInUcastPkts + mib_ptr->ifInNUcastPkts;
		network_io_ptr[interfaces].opackets = mib_ptr->ifOutUcastPkts + mib_ptr->ifOutNUcastPkts;
		network_io_ptr[interfaces].ierrors = mib_ptr->ifInErrors;
		network_io_ptr[interfaces].oerrors = mib_ptr->ifOutErrors;
#ifdef HPUX11
		mib_Dot3_ptr = (mib_Dot3StatsEntry *) (void *) ((int) mib_ptr + sizeof (mib_ifEntry));
		network_io_ptr[interfaces].collisions = mib_Dot3_ptr->dot3StatsSingleCollisionFrames
						      + mib_Dot3_ptr->dot3StatsMultipleCollisionFrames;
#else
		network_io_ptr[interfaces].collisions = 0; /* currently unknown */
#endif
		network_io_ptr[interfaces].systime = time(NULL);
		++interfaces;

		detach_req = (dl_detach_req_t*) ctrl_area;
		detach_req->dl_primitive = DL_DETACH_REQ;
		ctrl_buf.len = sizeof(dl_detach_req_t);
		if (putmsg(fd, &ctrl_buf, 0, 0) < 0) {
			unsigned ppa_lan_number = ppa_info[count].dl_ppa;
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_PUTMSG, "DL_DETACH_REQ ppa %u", ppa_lan_number);
		}

		ctrl_area[0] = 0;
		flags = 0;
		if (getmsg(fd, &ctrl_buf, &data_buf, &flags) < 0) {
			unsigned ppa_lan_number = ppa_info[count].dl_ppa;
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_GETMSG, "DL_DETACH_REQ ppa %u", ppa_lan_number);
		}
	}

	close(fd);

	free(ctrl_area);
	free(data_area);
	free(ppa_area);
#elif defined(ALLBSD)
	if (getifaddrs(&net) != 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_GETIFADDRS, NULL);
	}

	for( net_ptr=net; net_ptr != NULL; net_ptr = net_ptr->ifa_next ) {
		if( net_ptr->ifa_addr->sa_family != AF_LINK )
			continue;

		++interfaces;
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP

	VECTOR_UPDATE(network_io_stats_vector_ptr, interfaces, network_io_ptr, sg_network_io_stats);

	for( interfaces = 0, net_ptr = net; net_ptr != NULL; net_ptr = net_ptr->ifa_next ) {
		if( net_ptr->ifa_addr->sa_family != AF_LINK )
			continue;

		if( sg_update_string(&network_io_ptr[interfaces].interface_name,
				     net_ptr->ifa_name) != SG_ERROR_NONE ) {
			RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
		}
		net_data = (struct if_data *)net_ptr->ifa_data;
		network_io_ptr[interfaces].rx = net_data->ifi_ibytes;
		network_io_ptr[interfaces].tx = net_data->ifi_obytes;
		network_io_ptr[interfaces].ipackets = net_data->ifi_ipackets;
		network_io_ptr[interfaces].opackets = net_data->ifi_opackets;
		network_io_ptr[interfaces].ierrors = net_data->ifi_ierrors;
		network_io_ptr[interfaces].oerrors = net_data->ifi_oerrors;
		network_io_ptr[interfaces].collisions = net_data->ifi_collisions;
		network_io_ptr[interfaces].systime = time(NULL);

		++interfaces;
	}
	freeifaddrs(net);
#elif defined(AIX)
	/* check how many perfstat_netinterface_t structures are available */
	interfaces = perfstat_netinterface(NULL, NULL, sizeof(perfstat_netinterface_t), 0);

	/* allocate enough memory for all the structures */
	statp = calloc(interfaces, sizeof(perfstat_netinterface_t));
	if( NULL == statp ) {
		RETURN_WITH_SET_ERROR("network", SG_ERROR_MALLOC, "sg_get_network_io_stats");
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(statp);

	/* set name to first interface */
	strcpy(first.name, FIRST_NETINTERFACE);

	/* ask to get all the structures available in one call */
	/* return code is number of structures returned */
	interfaces = perfstat_netinterface(&first, statp, sizeof(perfstat_netinterface_t), interfaces);
	if( -1 == interfaces ) {
		free(statp);
		RETURN_WITH_SET_ERROR("network", SG_ERROR_SYSCTLBYNAME, "perfstat_netinterface");
	}

	VECTOR_UPDATE(network_io_stats_vector_ptr, interfaces, network_io_ptr, sg_network_io_stats);

	/* print statistics for each of the interfaces */
	for (i = 0; i < interfaces; ++i) {
		if (sg_update_string(&network_io_ptr[i].interface_name, statp[i].name) != SG_ERROR_NONE ) {
			free(statp);
			RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
		}
		network_io_ptr[i].tx = statp[i].obytes;
		network_io_ptr[i].rx = statp[i].ibytes;
		network_io_ptr[i].opackets = statp[i].opackets;
		network_io_ptr[i].ipackets = statp[i].ipackets;
		network_io_ptr[i].oerrors = statp[i].oerrors;
		network_io_ptr[i].ierrors = statp[i].ierrors;
		network_io_ptr[i].collisions = statp[i].collisions;
		network_io_ptr[i].systime = time(NULL);
	}

	free(statp);
#elif defined(SOLARIS)
	if ((kc = kstat_open()) == NULL) {
		RETURN_WITH_SET_ERROR("network", SG_ERROR_KSTAT_OPEN, NULL);
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP kstat_close(kc);

	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
		if (strcmp(ksp->ks_class, "net") == 0) {
			kstat_read(kc, ksp, NULL);

#ifdef SOL7
#       define LRX "rbytes"
#       define LTX "obytes"
#       define LIPACKETS "ipackets"
#       define LOPACKETS "opackets"
#       define VALTYPE value.ui32
#else
#       define LRX "rbytes64"
#       define LTX "obytes64"
#       define LIPACKETS "ipackets64"
#       define LOPACKETS "opackets64"
#       define VALTYPE value.ui64
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

			VECTOR_UPDATE(network_io_stats_vector_ptr, interfaces + 1, network_io_ptr, sg_network_io_stats);

			/* Read interface name */
			if (SG_ERROR_NONE != sg_update_string(&network_io_ptr[interfaces].interface_name, ksp->ks_name)) {
				kstat_close(kc);
				RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
			}

			/* Finish reading rx */
			network_io_ptr[interfaces].rx=knp->VALTYPE;

			/* Read tx */
			if((knp=kstat_data_lookup(ksp, LTX))==NULL)
				continue;
			network_io_ptr[interfaces].tx=knp->VALTYPE;

			/* Read ipackets */
			if((knp=kstat_data_lookup(ksp, LIPACKETS))==NULL)
				continue;
			network_io_ptr[interfaces].ipackets=knp->VALTYPE;

			/* Read opackets */
			if((knp=kstat_data_lookup(ksp, LOPACKETS))==NULL)
				continue;
			network_io_ptr[interfaces].opackets=knp->VALTYPE;

			/* Read ierrors */
			if((knp=kstat_data_lookup(ksp, "ierrors"))==NULL)
				continue;
			network_io_ptr[interfaces].ierrors=knp->value.ui32;

			/* Read oerrors */
			if((knp=kstat_data_lookup(ksp, "oerrors"))==NULL)
				continue;
			network_io_ptr[interfaces].oerrors=knp->value.ui32;

			/* Read collisions */
			if((knp=kstat_data_lookup(ksp, "collisions"))==NULL)
				continue;
			network_io_ptr[interfaces].collisions=knp->value.ui32;


			/* Store systime */
			network_io_ptr[interfaces].systime=time(NULL);

			++interfaces;
		}
	}

	kstat_close(kc);
#elif defined(LINUX)
	f=fopen("/proc/net/dev", "r");
	if(f==NULL) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_OPEN, "/proc/net/dev");
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP fclose(f);

	/* read the 2 lines.. Its the title, so we dont care :) */
	if( ( NULL == fgets(line, sizeof(line), f) ) ||
	    ( NULL == fgets(line, sizeof(line), f) ) ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_PARSE, "/proc/net/dev");
	}

	while((fgets(line, sizeof(line), f)) != NULL){
		if((regexec(&network_io_rx, line, RX_MATCH_COUNT, line_match, 0))!=0){
			continue;
		}

		VECTOR_UPDATE(network_io_stats_vector_ptr, interfaces + 1, network_io_ptr, sg_network_io_stats);

		if( network_io_ptr[interfaces].interface_name != NULL ) {
			free(network_io_ptr[interfaces].interface_name);
			network_io_ptr[interfaces].interface_name = NULL;
		}

		network_io_ptr[interfaces].interface_name = sg_get_string_match(line, &line_match[1]);
		network_io_ptr[interfaces].rx = sg_get_ll_match(line, &line_match[2]);
		network_io_ptr[interfaces].tx = sg_get_ll_match(line, &line_match[5]);
		network_io_ptr[interfaces].ipackets = sg_get_ll_match(line, &line_match[3]);
		network_io_ptr[interfaces].opackets = sg_get_ll_match(line, &line_match[6]);
		network_io_ptr[interfaces].ierrors = sg_get_ll_match(line, &line_match[4]);
		network_io_ptr[interfaces].oerrors = sg_get_ll_match(line, &line_match[7]);
		network_io_ptr[interfaces].collisions = sg_get_ll_match(line, &line_match[8]);
		network_io_ptr[interfaces].systime = time(NULL);

		++interfaces;
	}

	fclose(f);
#elif defined(WIN32)
	if((if_table = win32_get_devices()) == NULL) {
		RETURN_WITH_SET_ERROR("network", SG_ERROR_DEVICES, "win32_get_devices");
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(if_table);

	VECTOR_UPDATE(network_io_stats_vector_ptr, interfaces + 1, network_io_ptr, sg_network_io_stats);

	for ( i = 0; i < if_table->dwNumEntries; ++i ) {
		if_row = if_table->table[i];

		if(SG_ERROR_NONE != sg_update_string(&network_io_ptr[i].interface_name, if_row.bDescr) ) {
			free(if_table);
			RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
		}

		network_io_ptr[i].tx = if_row.dwOutOctets;
		network_io_ptr[i].rx = if_row.dwInOctets;
		network_io_ptr[i].ipackets = if_row.dwInUcastPkts + if_row.dwInNUcastPkts;
		network_io_ptr[i].opackets = if_row.dwOutUcastPkts + if_row.dwOutNUcastPkts;
		network_io_ptr[i].ierrors = if_row.dwInErrors;
		network_io_ptr[i].oerrors = if_row.dwOutErrors;
		network_io_ptr[i].collisions = 0; /* can't do that */
		network_io_ptr[i].systime = time(NULL);

		++interfaces;
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
	for (i=0; i < interfaces; ++i) {
		int no = 2, j;
		for(j=i+1; j<interfaces; ++j) {
			if(strcmp(network_io_ptr[i].interface_name, network_io_ptr[j].interface_name) == 0) {
				if(snprintf(buf, sizeof(buf), " #%d", no) < 0) {
					break;
				}
				if(sg_concat_string(&network_io_ptr[i].interface_name, buf) != 0) {
					RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
				}

				++no;
			}
		}
	}
#else
	RETURN_WITH_SET_ERROR("network", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif

	return SG_ERROR_NONE;
}

MULTI_COMP_ACCESS(sg_get_network_io_stats,network,network_io,SG_NETWORK_IO_NOW_IDX)
MULTI_COMP_DIFF(sg_get_network_io_stats_diff,sg_get_network_io_stats,network,network_io,SG_NETWORK_IO_DIFF_IDX,SG_NETWORK_IO_NOW_IDX)

int
sg_network_io_compare_name(const void *va, const void *vb) {
	const sg_network_io_stats *a = va, *b = vb;
	return strcmp(a->interface_name, b->interface_name);
}

/* NETWORK INTERFACE STATS */

static sg_error
sg_get_network_iface_stats_int(sg_vector **network_iface_vector_ptr){
	sg_network_iface_stats *network_iface_stat;
	size_t ifaces = 0;
	time_t now = time(NULL);

#ifdef SOLARIS
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_named_t *knp;
	int sock;
#elif defined(ALLBSD)
	struct ifmediareq ifmed;
	struct ifaddrs *net, *net_ptr;
	struct ifreq ifr;
	int sock;
	int x;
#elif defined(LINUX)
	FILE *f;
	/* Horrible big enough, but it should be easily big enough */
	char line[8096];
	int sock;
#elif defined(WIN32)
	PMIB_IFTABLE if_table;
	MIB_IFROW if_row;
	int i,j,no;
	char buf[5];
#elif defined(AIX)
	int fd, n;
	ssize_t pagesize;
	struct ifreq *ifr, *lifr, iff;
	struct ifconf ifc;
#elif defined(HPUX)
	int fd;
	struct ifreq *ifr, *lifr, iff;
	struct ifconf ifc;
#endif

#ifdef ALLBSD
	if(getifaddrs(&net) != 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_GETIFADDRS, NULL);
	}

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_SOCKET, NULL);
	}

	for( net_ptr = net; net_ptr != NULL; net_ptr = net_ptr->ifa_next ) {
		if( net_ptr->ifa_addr->sa_family != AF_LINK )
			continue;

		++ifaces;
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP close(sock); freeifaddrs(net);

	VECTOR_UPDATE(network_iface_vector_ptr, ifaces, network_iface_stat, sg_network_iface_stats);

	for( ifaces = 0, net_ptr = net; net_ptr != NULL; net_ptr = net_ptr->ifa_next ) {
		if( net_ptr->ifa_addr->sa_family != AF_LINK )
			continue;


		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, net_ptr->ifa_name, sizeof(ifr.ifr_name));

		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
			continue;

		network_iface_stat[ifaces].up = ((ifr.ifr_flags & IFF_UP) != 0) ? 1 : 0;

		if (sg_update_string(&network_iface_stat[ifaces].interface_name,
				     net_ptr->ifa_name) != SG_ERROR_NONE ) {
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
		}

		network_iface_stat[ifaces].speed = 0;
		network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_UNKNOWN;

		network_iface_stat[ifaces].systime = now;

		memset(&ifmed, 0, sizeof(struct ifmediareq));
		sg_strlcpy(ifmed.ifm_name, net_ptr->ifa_name, sizeof(ifmed.ifm_name));
		if(ioctl(sock, SIOCGIFMEDIA, (caddr_t)&ifmed) == -1){
			/* Not all interfaces support the media ioctls. */
			goto skip;
		}

		/* We may need to change this if we start doing wireless devices too */
		if( (ifmed.ifm_active | IFM_ETHER) != ifmed.ifm_active ){
			/* Not a ETHER device */
			goto skip;
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
				network_iface_stat[ifaces].speed = 10;
				break;
			/* 100 Mbit connections */
			case(IFM_100_TX):
			case(IFM_100_FX):
			case(IFM_100_T4):
			case(IFM_100_VG):
			case(IFM_100_T2):
				network_iface_stat[ifaces].speed = 100;
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
				network_iface_stat[ifaces].speed = 1000;
				break;
			/* We don't know what it is */
			default:
				network_iface_stat[ifaces].speed = 0;
				break;
		}

		if( (ifmed.ifm_active | IFM_FDX) == ifmed.ifm_active ) {
			network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_FULL;
		}
		else if( (ifmed.ifm_active | IFM_HDX) == ifmed.ifm_active ) {
			network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_HALF;
		}
		else {
			network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_UNKNOWN;
		}

skip:
#ifdef SG_ENABLE_DEPRECATED
		network_iface_stat[ifaces].dup = network_iface_stat[ifaces].duplex;
#endif

		++ifaces;
	}
	freeifaddrs(net);
	close(sock);
#elif defined(AIX)
/*
 * Fix up for variable length of struct ifr
 */
#ifndef _SIZEOF_ADDR_IFREQ
#define _SIZEOF_ADDR_IFREQ(ifr) \
	(((ifr).ifr_addr.sa_len > sizeof(struct sockaddr)) \
	? sizeof(struct ifreq) - sizeof(struct sockaddr) + (ifr).ifr_addr.sa_len \
	: sizeof(struct ifreq))
#endif

	if( (pagesize = sysconf(_SC_PAGESIZE)) == -1 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_SYSCONF, "_SC_PAGESIZE");
	}

	if( (fd = socket (PF_INET, SOCK_DGRAM, 0)) == -1 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_SOCKET, NULL);
	}

	n = 2;
	ifr = (struct ifreq *)malloc( n*pagesize );
	if( NULL == ifr ) {
		close (fd);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_MALLOC, NULL);
	}
	bzero(&ifc, sizeof(ifc));
	ifc.ifc_req = ifr;
	ifc.ifc_len = n * pagesize;
	while( ( ioctl( fd, SIOCGIFCONF, &ifc ) == -1 ) ||
	       ( ((ssize_t)ifc.ifc_len) >= ( (n-1) * pagesize)) )
	{
		n *= 2;
		if( (n * pagesize) > INT_MAX ) {
			free( ifc.ifc_req );
			close (fd);
			RETURN_WITH_SET_ERROR_WITH_ERRNO_CODE("network", SG_ERROR_MALLOC, ERANGE, "SIOCGIFCONF");
		}
		ifr = (struct ifreq *)realloc( ifr, n*pagesize );
		if( NULL == ifr ) {
			free( ifc.ifc_req );
			close (fd);
			RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_MALLOC, NULL);
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

		++ifaces;
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP close(fd); free(ifc.ifc_req);

	VECTOR_UPDATE(network_iface_vector_ptr, ifaces, network_iface_stat, sg_network_iface_stats);

	for ( ifaces = 0, ifr = ifc.ifc_req;
	      ifr < lifr;
	      ifr = (struct ifreq *)&(((char *)ifr)[_SIZEOF_ADDR_IFREQ(*ifr)]) )
	{
		struct sockaddr *sa = &ifr->ifr_addr;

		if(sa->sa_family != AF_LINK)
			continue;

		memset(&iff, 0, sizeof(iff));
		strncpy(iff.ifr_name, ifr->ifr_name, sizeof(iff.ifr_name));

		if (ioctl(fd, SIOCGIFFLAGS, &iff) < 0)
			continue;

		network_iface_stat[ifaces].up = ((iff.ifr_flags & IFF_UP) != 0) ? 1 : 0;
		if (sg_update_string(&network_iface_stat[ifaces].interface_name,
				     ifr->ifr_name) != SG_ERROR_NONE ) {
			close (fd);
			free( ifc.ifc_req );
			RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
		}

		network_iface_stat[ifaces].speed = 0;
		/* XXX */
#ifdef SG_ENABLE_DEPRECATED
		network_iface_stat[ifaces].dup =
#endif
		network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_UNKNOWN;
		network_iface_stat[ifaces].systime = now;
		++ifaces;
	}

	close (fd);
	free( ifc.ifc_req );
#elif defined(HPUX)
/*
 * Fix up for variable length of struct ifr
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
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_SOCKET, NULL);
	}

	bzero(&ifc, sizeof(ifc));
	if (ioctl (fd, SIOCGIFNUM, &ifc) != -1) {
		ifr = calloc(sizeof (*ifr), ifc.ifc_len);
		if( NULL == ifr ) {
			close (fd);
			RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_MALLOC, NULL);
		}
		ifc.ifc_req = ifr;
		ifc.ifc_len *= sizeof (*ifr);
		if (ioctl (fd, SIOCGIFCONF, &ifc) == -1) {
			free (ifr);
			close (fd);
			RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_GETIFADDRS, NULL);
		}
	}
	else {
		close (fd);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_GETIFADDRS, NULL);
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP close(fd); free(ifc.ifc_req);

	lifr = (struct ifreq *)&ifc.ifc_buf[ifc.ifc_len];
	for ( ifr = ifc.ifc_req;
	      ifr < lifr;
	      ifr = (struct ifreq *)(((char *)ifr) + _SIZEOF_ADDR_IFREQ(*ifr)) )
	{
		VECTOR_UPDATE(network_iface_vector_ptr, ifaces + 1, network_iface_stat, sg_network_iface_stats);

		bzero(&iff, sizeof(iff));
		strncpy(iff.ifr_name, ifr->ifr_name, sizeof(iff.ifr_name));

		if (ioctl(fd, SIOCGIFFLAGS, &iff) < 0) {
			continue;
		}

		network_iface_stat[ifaces].up = ((iff.ifr_flags & IFF_UP) != 0) ? 1 : 0;
		if (sg_update_string(&network_iface_stat[ifaces].interface_name,
				     ifr->ifr_name) != SG_ERROR_NONE ) {
			close (fd);
			free( ifc.ifc_req );
			RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
		}

		/**
		 * for physical devices we now could ask /dev/dlpi for speed and type,
		 * but for monitoring we don't care
		 */
		network_iface_stat[ifaces].speed = 0;
#ifdef SG_ENABLE_DEPRECATED
		network_iface_stat[ifaces].dup =
#endif
		network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_UNKNOWN;
		network_iface_stat[ifaces].systime = now;
		++ifaces;
	}

	close (fd);
	free( ifc.ifc_req );
#elif defined(SOLARIS)
	if ((kc = kstat_open()) == NULL) {
		RETURN_WITH_SET_ERROR("network", SG_ERROR_KSTAT_OPEN, NULL);
	}

	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) {
		kstat_close(kc);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_SOCKET, NULL);
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP close(sock); kstat_close(kc);

	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
		if (strcmp(ksp->ks_class, "net") == 0) {
			struct ifreq ifr;

			kstat_read(kc, ksp, NULL);

			strncpy(ifr.ifr_name, ksp->ks_name, sizeof(ifr.ifr_name));
			if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
				/* Not a network interface. */
				continue;
			}

			VECTOR_UPDATE(network_iface_vector_ptr, ifaces + 1, network_iface_stat, sg_network_iface_stats);

			if (sg_update_string(&network_iface_stat[ifaces].interface_name,
					     ksp->ks_name) != SG_ERROR_NONE ) {
				VECTOR_UPDATE_ERROR_CLEANUP
				RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
			}

			if ((ifr.ifr_flags & IFF_UP) != 0) {
				if ((knp = kstat_data_lookup(ksp, "link_up")) != NULL) {
					/* take in to account if link
					 * is up as well as interface */
					if (knp->value.ui32 != 0u) {
						network_iface_stat[ifaces].up = 1;
					}
					else {
						network_iface_stat[ifaces].up = 0;
					}
				}
				else {
					/* maintain compatibility */
					network_iface_stat[ifaces].up = 1;
				}
			}
			else {
				network_iface_stat[ifaces].up = 0;
			}

			if ((knp = kstat_data_lookup(ksp, "ifspeed")) != NULL) {
				network_iface_stat[ifaces].speed = knp->value.ui64 / (1000 * 1000);
			}
			else {
				network_iface_stat[ifaces].speed = 0;
			}

			network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_UNKNOWN;
			if ((knp = kstat_data_lookup(ksp, "link_duplex")) != NULL) {
				switch (knp->value.ui32) {
				case 1:
					network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_HALF;
					break;
				case 2:
					network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_FULL;
					break;
				}
			}

#ifdef SG_ENABLE_DEPRECATED
			network_iface_stat[ifaces].dup = network_iface_stat[ifaces].duplex;
#endif
			network_iface_stat[ifaces].systime = now;

			++ifaces;
		}
	}

	close(sock);
	kstat_close(kc);
#elif defined(LINUX)
	f = fopen("/proc/net/dev", "r");
	if(f == NULL) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_OPEN, "/proc/net/dev");
	}

	/* Setup stuff so we can do the ioctl to get the info */
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fclose(f);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("network", SG_ERROR_SOCKET, NULL);
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP close(sock); fclose(f);

	/* Ignore first 2 lines.. Just headings */
	if((fgets(line, sizeof(line), f)) == NULL) {
		VECTOR_UPDATE_ERROR_CLEANUP
		RETURN_WITH_SET_ERROR("network", SG_ERROR_PARSE, NULL);
	}
	if((fgets(line, sizeof(line), f)) == NULL) {
		VECTOR_UPDATE_ERROR_CLEANUP
		RETURN_WITH_SET_ERROR("network", SG_ERROR_PARSE, NULL);
	}

	while((fgets(line, sizeof(line), f)) != NULL){
		char *name, *ptr;
		struct ifreq ifr;
		struct ethtool_cmd ethcmd;
		int err;

		/* Get the interface name */
		ptr = strchr(line, ':');
		if (ptr == NULL)
			continue;
		*ptr='\0';
		name = line;
		while(isspace(*(name))){
			++name;
		}

		memset(&ifr, 0, sizeof ifr);
		strncpy(ifr.ifr_name, name, sizeof ifr.ifr_name);

		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
			continue;
		}

		/* We have a good interface to add */
		VECTOR_UPDATE(network_iface_vector_ptr, ifaces + 1, network_iface_stat, sg_network_iface_stats);

		if (sg_update_string(&network_iface_stat[ifaces].interface_name, name) != SG_ERROR_NONE ) {
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
		}
		network_iface_stat[ifaces].up = ((ifr.ifr_flags & IFF_UP) != 0) ? 1 : 0;

		memset(&ethcmd, 0, sizeof ethcmd);
		ethcmd.cmd = ETHTOOL_GSET;
		ifr.ifr_data = (caddr_t) &ethcmd;

		err = ioctl(sock, SIOCETHTOOL, &ifr);
		if (err == 0) {
			network_iface_stat[ifaces].speed = ethcmd.speed;

			switch (ethcmd.duplex) {
			case DUPLEX_FULL:
				network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_FULL;
				break;
			case DUPLEX_HALF:
				network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_HALF;
				break;
			default:
				network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_UNKNOWN;
			}
		}
		else {
			/* Not all interfaces support the ethtool ioctl. */
			network_iface_stat[ifaces].speed = 0;
#ifdef SG_ENABLE_DEPRECATED
			network_iface_stat[ifaces].dup =
#endif
			network_iface_stat[ifaces].duplex = SG_IFACE_DUPLEX_UNKNOWN;
		}

		network_iface_stat[ifaces].systime = now;

		++ifaces;
	}
	close(sock);
	fclose(f);
#elif defined(WIN32)
	if((if_table = win32_get_devices()) == NULL) {
		RETURN_WITH_SET_ERROR("network", SG_ERROR_DEVICES, "network interfaces");
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(if_table);

	VECTOR_UPDATE(network_iface_vector_ptr, if_table->dwNumEntries, network_iface_stat, sg_network_iface_stats);

	for( i = 0; i < if_table->dwNumEntries; ++i ) {
		if_row = if_table->table[i];

		if(sg_update_string(&network_iface_stat[i].interface_name,
					if_row.bDescr) != SG_ERROR_NONE ) {
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
		}
		network_iface_stat[i].speed = if_row.dwSpeed /1000000;

		if( ( if_row.dwOperStatus == MIB_IF_OPER_STATUS_CONNECTED ||
		      if_row.dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL ) &&
		    if_row.dwAdminStatus == 1 ) {
			network_iface_stat[i].up = 1;
		}
		else {
			network_iface_stat[i].up = 0;
		}

		network_iface_stat[i].systime = now;

		++ifaces;
	}
	free(if_table);

	/* again with the renumbering */
	for (i=0; i < ifaces; ++i) {
		int no = 2, j;
		for(j=i+1; j<ifaces; ++j) {
			if(strcmp(network_io_ptr[i].interface_name, network_io_ptr[j].interface_name) == 0) {
				if(snprintf(buf, sizeof(buf), " #%d", no) < 0) {
					break;
				}
				if(sg_concat_string(&network_io_ptr[i].interface_name, buf) != 0) {
					RETURN_FROM_PREVIOUS_ERROR( "network", sg_get_error() );
				}

				++no;
			}
		}
	}
#else
	RETURN_WITH_SET_ERROR("network", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif

#ifdef SG_ENABLE_DEPRECATED
	network_iface_stat_ptr->dup = network_iface_stat_ptr->duplex;
#endif

	return SG_ERROR_NONE;
}

MULTI_COMP_ACCESS(sg_get_network_iface_stats,network,network_iface,SG_NETWORK_IFACE_IDX)

int
sg_network_iface_compare_name(const void *va, const void *vb) {
	const sg_network_iface_stats *a = va, *b = vb;
	return strcmp(a->interface_name, b->interface_name);
}
