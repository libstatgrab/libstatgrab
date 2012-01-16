/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2011 i-scream
 * Copyright (C) 2010,2011 Jens Rehsack
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

#ifdef SOLARIS
#define VALID_FS_TYPES {"ufs", "tmpfs", "vxfs", "nfs", "zfs", "lofs"}
#elif defined(LINUX)
#define VALID_FS_TYPES {"adfs", "affs", "befs", "bfs", "btrfs", "cifs", \
			"efs", "ext2", "ext3", "ext4", "vxfs", "hfs", \
			"hfsplus", "hpfs", "jffs", "jffs2", "jfs", "minix", \
			"msdos", "nfs", "ntfs", "ocfs", "qnx4", "ramfs", \
			"rootfs", "reiserfs", "sysv", "tmpfs", "v7", "udf", \
			"ufs", "umsdos", "vfat", "xfs" }
#elif defined(CYGWIN)
#define VALID_FS_TYPES {"user"}
#elif defined(FREEBSD)
/*#define VALID_FS_TYPES {"hpfs", "msdosfs", "ntfs", "udf", "ext2fs", \
			"ufs", "mfs", "nfs", "zfs", "tmpfs", "reiserfs", \
			"xfs"}*/
#elif defined(DFBSD)
#define VALID_FS_TYPES {"hammer", "msdosfs", "ntfs", "udf", "ext2fs", \
			"ufs", "mfs", "nfs", "tmpfs"}
#elif defined(NETBSD) || defined(OPENBSD)
#define VALID_FS_TYPES {"ffs", "mfs", "msdos", "lfs", "adosfs", "ext2fs", \
			"ntfs", "nfs"}
#elif defined(HPUX)
#define VALID_FS_TYPES {"vxfs", "hfs", "nfs"}
#elif defined(AIX)
#define VALID_FS_TYPES {		\
	"jfs2",		/*  0 */	\
	"namefs",	/*  1 */	\
	"nfs",		/*  2 */	\
	"jfs",		/*  3 */	\
	NULL,		/*  4 */	\
	"cdrom",	/*  5 */	\
	"procfs",	/*  6 */	\
	NULL,		/*  7 */	\
	NULL,		/*  8 */	\
	NULL,		/*  9 */	\
	NULL,		/* 10 */	\
	NULL,		/* 11 */	\
	NULL,		/* 12 */	\
	NULL,		/* 13 */	\
	NULL,		/* 14 */	\
	NULL,		/* 15 */	\
	"sfs",		/* 16 */	\
	"cachefs",	/* 17 */	\
	"nfs3",		/* 18 */	\
	"autofs",	/* 19 */	\
	NULL,		/* 20 */	\
	NULL,		/* 21 */	\
	NULL,		/* 22 */	\
	NULL,		/* 23 */	\
	NULL,		/* 24 */	\
	NULL,		/* 25 */	\
	NULL,		/* 26 */	\
	NULL,		/* 27 */	\
	NULL,		/* 28 */	\
	NULL,		/* 29 */	\
	NULL,		/* 30 */	\
	NULL,		/* 31 */	\
	"vxfs",		/* 32 */	\
	"vxodm",	/* 33 */	\
	"udf",		/* 34 */	\
	"nfs4",		/* 35 */	\
	"rfs4",		/* 36 */	\
	"cifs"		/* 37 */	\
}
static char **sys_fs_types;
static size_t num_sys_fs_types;
#ifndef HAVE_DECL_MNTCTL
extern int mntctl(int, int, void *);
#endif
#elif defined(WIN32)
#define VALID_FS_TYPES {"NTFS", "FAT", "FAT32"};
#endif

static size_t num_valid_file_systems = 0;
static char **valid_file_systems = NULL;

static void
sg_fs_stats_item_init(sg_fs_stats *d) {

	d->device_name = NULL;
	d->fs_type = NULL;
	d->mnt_point = NULL;
}

static sg_error
sg_fs_stats_item_copy(sg_fs_stats *d, const sg_fs_stats *s) {

	if( SG_ERROR_NONE != sg_update_string(&d->device_name, s->device_name) ||
	    SG_ERROR_NONE != sg_update_string(&d->fs_type, s->fs_type) ||
	    SG_ERROR_NONE != sg_update_string(&d->mnt_point, s->mnt_point) ) {
		RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
	}

	d->device_type = s->device_type;
	d->size = s->size;
	d->used = s->used;
	d->free = s->free;
	d->avail = s->avail;
	d->total_inodes = s->total_inodes;
	d->used_inodes = s->used_inodes;
	d->free_inodes = s->free_inodes;
	d->avail_inodes = s->avail_inodes;
	d->io_size = s->io_size;
	d->block_size = s->block_size;
	d->total_blocks = s->total_blocks;
	d->free_blocks = s->free_blocks;
	d->used_blocks = s->used_blocks;
	d->avail_blocks = s->avail_blocks;
	d->systime = s->systime;

	return SG_ERROR_NONE;
}

static sg_error
sg_fs_stats_item_compute_diff(const sg_fs_stats *s, sg_fs_stats *d) {

	d->size -= s->size;
	d->used -= s->used;
	d->avail -= s->avail;
	d->total_inodes -= s->total_inodes;
	d->used_inodes -= s->used_inodes;
	d->free_inodes -= s->free_inodes;
	d->avail_inodes -= s->avail_inodes;
	d->io_size -= s->io_size;
	d->block_size -= s->block_size;
	d->total_blocks -= s->total_blocks;
	d->free_blocks -= s->free_blocks;
	d->used_blocks -= s->used_blocks;
	d->avail_blocks -= s->avail_blocks;

	return SG_ERROR_NONE;
}

static int
sg_fs_stats_item_compare(const sg_fs_stats *a, const sg_fs_stats *b) {

	int rc;

	rc = strcmp(a->device_name, b->device_name);
	if( 0 != rc )
		return rc;

	rc = strcmp(a->mnt_point, b->mnt_point);
	if( 0 != rc )
		return rc;

	rc = strcmp(a->fs_type, b->fs_type);
	if( 0 != rc )
		return rc;

	return 0;
}

static void
sg_fs_stats_item_destroy(sg_fs_stats *d) {
	free(d->device_name);
	free(d->fs_type);
	free(d->mnt_point);
}

static void sg_disk_io_stats_item_init(sg_disk_io_stats *d) {
	d->disk_name = NULL;
}

static sg_error
sg_disk_io_stats_item_copy(sg_disk_io_stats *d, const sg_disk_io_stats *s) {

	if( SG_ERROR_NONE != sg_update_string(&d->disk_name, s->disk_name) ) {
		RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
	}

	d->read_bytes = s->read_bytes;
	d->write_bytes = s->write_bytes;
	d->systime = s->systime;

	return SG_ERROR_NONE;
}

static sg_error
sg_disk_io_stats_item_compute_diff(const sg_disk_io_stats *s, sg_disk_io_stats *d) {

	d->read_bytes -= s->read_bytes;
	d->write_bytes -= s->write_bytes;
	d->systime -= s->systime;

	return SG_ERROR_NONE;
}

static int
sg_disk_io_stats_item_compare(const sg_disk_io_stats *a, const sg_disk_io_stats *b) {
	return strcmp(a->disk_name, b->disk_name);
}

static void
sg_disk_io_stats_item_destroy(sg_disk_io_stats *d) {
	free(d->disk_name);
}

VECTOR_INIT_INFO_FULL_INIT(sg_fs_stats);
VECTOR_INIT_INFO_FULL_INIT(sg_disk_io_stats);

static int
cmp_valid_fs(const void *va, const void *vb) {
	const char *a = * (char * const *)va;
	const char *b = * (char * const *)vb;

	if( a && b )
		return strcasecmp(a, b);
	/* force NULL sorted at end */
	if( a && !b )
		return -1;
	if( !a && b )
		return 1;
	return 0;
}

static sg_error
init_valid_fs_types(void) {
	sg_error errc = SG_ERROR_NONE;
	size_t i;

#define FILL_VALID_FS

#if defined(HAVE_STRUCT_XVFSCONF) && defined(HAVE_SYSCTLBYNAME)
	struct xvfsconf *vfsp;
	size_t buflen;

	/* XXX use sysctl(mib[]) instead of sysctlbyname() - better testable in configure */
	if( sysctlbyname("vfs.conflist", NULL, &buflen, NULL, 0) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_SYSCTLBYNAME, "vfs.conflist");
	}
	if( NULL == (vfsp = sg_malloc(buflen) ) ) {
		RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
	}
	if( sysctlbyname("vfs.conflist", vfsp, &buflen, NULL, 0) < 0 )
	{
		free(vfsp);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_SYSCTLBYNAME, "vfs.conflist");
	}
	num_valid_file_systems = buflen / sizeof(*vfsp);

#define CLEANUP_INIT_VALID_FS free(vfsp);
#define CUR_VALID_FS vfsp[i].vfc_name

#elif defined(HAVE_STRUCT_VFSCONF) && defined(VFS_CONF)
	const char *int_valid_fs[] = VALID_FS_TYPES;
	struct vfsconf *vfsp = 0;
	size_t buflen;
	int nbvfs = 0, k;
	int mib[4] = { CTL_VFS, VFS_GENERIC, VFS_MAXTYPENUM };

	buflen = sizeof(nbvfs);
	if( sysctl(mib, 3, &nbvfs, &buflen, (void *)0, (size_t)0) < 0 ) {
		SET_ERROR_WITH_ERRNO("disk", SG_ERROR_SYSCTL, "vfs.generic.maxtypenum");
		free(vfsp);
		vfsp = NULL; nbvfs = 0;
		num_valid_file_systems = lengthof(int_valid_fs);
	}
	vfsp = calloc( nbvfs, sizeof(*vfsp) );
	if( NULL == vfsp ) {
		SET_ERROR_WITH_ERRNO("disk", SG_ERROR_MALLOC, "calloc for vfs.generic.conf");
		free(vfsp);
		vfsp = NULL; nbvfs = 0;
		num_valid_file_systems = lengthof(int_valid_fs);
	}
	if( vfsp ) {
		mib[2] = VFS_CONF;
		for( k = 0; k < nbvfs; ++k ) {
			mib[3] = k + 1;
			buflen = sizeof(vfsp[k]);
			if( sysctl(mib, 4, &vfsp[k], &buflen, (void *)0, (size_t)0) < 0 ) {
				if (errno == EOPNOTSUPP) {
					continue;
				}

				SET_ERROR_WITH_ERRNO("disk", SG_ERROR_SYSCTL, "vfs.generic.conf.%d", k+1);
				free(vfsp);
				vfsp = NULL; nbvfs = 0;
				num_valid_file_systems = lengthof(int_valid_fs);
				break;
			}
		}

		if( vfsp ) {
			num_valid_file_systems = nbvfs;
		}
	}

#define CLEANUP_INIT_VALID_FS free(vfsp);
#define CUR_VALID_FS (vfsp ? vfsp[i].vfc_name : int_valid_fs[i] )
#define CAN_SHRINK_VALID_FS

#elif defined(HAVE_SYSCTLBYNAME) && defined(CTL_VFS_NAMES)
	size_t fsnamebuflen;
	const char *int_valid_fs[] = VALID_FS_TYPES;
	char *fsnamebuf = 0, *fsname = 0;

	if( sysctlbyname("vfs.generic.fstypes", NULL, &fsnamebuflen, NULL, 0) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_SYSCTLBYNAME, "vfs.generic.fstypes");
	}
	if( fsnamebuflen > 1 ) {
		if( NULL == (fsnamebuf = sg_malloc(fsnamebuflen + 1) ) ) {
			RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
		}
		if( sysctlbyname("vfs.generic.fstypes", fsnamebuf, &fsnamebuflen, NULL, 0) < 0 )
		{
			free(fsnamebuf);
			RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_SYSCTLBYNAME, "vfs.generic.fstypes");
		}

		fsname = fsnamebuf;
		num_valid_file_systems = 1;
		while( ((size_t)(fsname - fsnamebuf)) < fsnamebuflen ) {
			if( *fsname == ' ' ) {
				*fsname = '\0';
				++num_valid_file_systems;
			}
			++fsname;
		}

		fsname = fsnamebuf;
	}

#define CLEANUP_INIT_VALID_FS do { if( fsnamebuf ) free(fsnamebuf); } while(0);
#define CUR_VALID_FS fsnamebuf ? (0 == i) ? fsname : (fsname += strlen(fsname)+1) : int_valid_fs[i]

#elif defined(HAVE_SYSFS) && defined(FSTYPSZ) && defined(GETNFSTYP) && defined(GETFSTYP)
	const char *int_valid_fs[] = VALID_FS_TYPES;
	int max_fs_type;
	struct {
		char name[FSTYPSZ];
	} *valid_sysfs = NULL;

	max_fs_type = sysfs(GETNFSTYP);
	if( max_fs_type > 0 ) {
		valid_sysfs = calloc( max_fs_type, sizeof(*valid_sysfs) );
		if( NULL != valid_sysfs ) {
			int i;

			for( i = 0, num_valid_file_systems = 0; i < max_fs_type; ++i, ++num_valid_file_systems ) {
				sysfs( GETFSTYP, i, &valid_sysfs[i] );
			}
		}
		else {
			num_valid_file_systems = lengthof(int_valid_fs);
		}
	}
	else {
		num_valid_file_systems = lengthof(int_valid_fs);
	}

#define CLEANUP_INIT_VALID_FS free(valid_sysfs);
#define CUR_VALID_FS (valid_sysfs ? valid_sysfs[i].name : int_valid_fs[i])
#define CAN_SHRINK_VALID_FS

#elif defined(AIX)
	FILE *fh;
	const char *int_valid_fs[] = VALID_FS_TYPES;

	num_sys_fs_types = num_valid_file_systems = lengthof(int_valid_fs);

	sys_fs_types = calloc( num_valid_file_systems, sizeof(sys_fs_types[0]) );
	if( NULL == sys_fs_types ) {
		num_sys_fs_types = num_valid_file_systems = 0;
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_MALLOC, "init_valid_fs_types");
	}

	for (i = 0; i < num_valid_file_systems; ++i) {
		if( SG_ERROR_NONE != ( errc = sg_update_string( &sys_fs_types[i], int_valid_fs[i] ) ) ) {
			num_sys_fs_types = num_valid_file_systems = 0;
			free(sys_fs_types);
			sys_fs_types = 0;
			RETURN_FROM_PREVIOUS_ERROR( "disk", errc );
		}
	}

	/* XXX probably better use setvfsent(),getvfsent(),endvfsent() */
	fh = fopen("/etc/vfs", "r");
	if( 0 != fh ) {
		char line[1024];

		while( fgets( line, sizeof(line), fh ) ) {
			char fstype[16], mnt_helper[PATH_MAX], filesys_helper[PATH_MAX], remote[16];
			if( line[0] < 'a' || line[0] > 'z' ) {
				continue; /* not a valid fstype */
			}
			if( sscanf(line, "%s %lu %s %s %s", fstype, &i, mnt_helper, filesys_helper, remote)  < 4 ) {
				continue; /* couldn't parse */
			}
			if( i >= num_sys_fs_types ) {
				/* fs-index starts at 0, so always alloc one more than index */
				char **tmp = sg_realloc( sys_fs_types, sizeof(sys_fs_types[0]) * (i + 1) );
				if( NULL == tmp ) {
					RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
				}
				sys_fs_types = tmp;
				memset( &sys_fs_types[num_sys_fs_types], 0, (i + 1 - num_sys_fs_types) * sizeof(sys_fs_types[0]) );
				num_valid_file_systems = num_sys_fs_types = i + 1;
			}
			if( SG_ERROR_NONE != ( errc = sg_update_string( &sys_fs_types[i], fstype ) ) ) {
				fclose(fh);
				free(sys_fs_types);
				num_sys_fs_types = num_valid_file_systems = 0;
				sys_fs_types = 0;
				RETURN_FROM_PREVIOUS_ERROR( "disk", errc );
			}
		}

		fclose(fh);
	}

#define CLEANUP_INIT_VALID_FS
#define CUR_VALID_FS sys_fs_types[i]
#define CAN_SHRINK_VALID_FS

#elif defined(LINUX)

#undef FILL_VALID_FS

	FILE *fh;
	const char *int_valid_fs[] = VALID_FS_TYPES;

	if( NULL != ( fh = fopen( "/proc/filesystems", "r" ) ) ) {
		regex_t re;
		const char *proc_fs_list_regex = "^([[:alnum:]]+)?[[:blank:]]+([[:alnum:]]+)";
		int rc;
		char line[4096];
		regmatch_t pmatch[3]; /* to fit for entire match, 1st and 2nd bracket content */

		i = 0;
		num_valid_file_systems = 15;
		valid_file_systems = calloc( num_valid_file_systems + 1, sizeof(char *) );

		if( ( rc = regcomp(&re, proc_fs_list_regex, REG_EXTENDED) ) != 0 ) {
			WARN_LOG_FMT( "disk", "error parsing regex '%s': %d", proc_fs_list_regex, rc );
			free(valid_file_systems);
			fclose(fh);
			goto use_builtin_fs;
		}
		while( fgets( line, sizeof(line), fh ) ) {
			if( 0 == ( rc = regexec( &re, line, lengthof(pmatch), pmatch, 0 ) ) ) {
				char *fs_name = line + pmatch[2].rm_so;
				line[pmatch[2].rm_eo] = '\0';

				if( i >= num_valid_file_systems ) {
					/* valid_file_systems has NULL termination, so always alloc one more than index */
					char **tmp = sg_realloc( valid_file_systems, sizeof(tmp[0]) * ((num_valid_file_systems += 16) + 1) );
					if( NULL == tmp ) {
						regfree( &re );
						fclose( fh );
						free(valid_file_systems);
						num_valid_file_systems = 0;
						valid_file_systems = NULL;
						RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
					}
					valid_file_systems = tmp;
					memset( &valid_file_systems[i], 0, ((num_valid_file_systems + 1) - i) * sizeof(valid_file_systems[0]) );
				}

				if( SG_ERROR_NONE != ( errc = sg_update_string( &valid_file_systems[i], fs_name ) ) ) {
					regfree( &re );
					fclose( fh );
					free(valid_file_systems);
					num_valid_file_systems = 0;
					valid_file_systems = NULL;
					RETURN_FROM_PREVIOUS_ERROR( "disk", errc );
				}
				++i;
			}
		}
		regfree( &re );
		fclose( fh );
	}
	else {
use_builtin_fs:
		valid_file_systems = calloc( num_valid_file_systems + 1, sizeof(char *) );
		if( 0 == valid_file_systems ) {
			RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_MALLOC, "init_valid_fs_types");
		}
		for( i = 0; i < num_valid_file_systems; ++i ) {
			if( SG_ERROR_NONE != ( errc = sg_update_string( &valid_file_systems[i], int_valid_fs[i] ) ) ) {
				WARN_LOG_FMT("disk", "couldn't update index %d for list of valid file systems", i);
				RETURN_FROM_PREVIOUS_ERROR( "disk", errc );
			}
		}
	}
#else
	const char *int_valid_fs[] = VALID_FS_TYPES;
	num_valid_file_systems = lengthof(int_valid_fs);

#define CLEANUP_INIT_VALID_FS
#define CUR_VALID_FS int_valid_fs[i]

#endif /* sys-dependent initialization */

#ifdef FILL_VALID_FS
	valid_file_systems = calloc( num_valid_file_systems + 1, sizeof(*valid_file_systems) );
	if( 0 == valid_file_systems ) {
		CLEANUP_INIT_VALID_FS
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_MALLOC, "init_valid_fs_types");
	}
	for( i = 0; i < num_valid_file_systems; ++i ) {
		const char *cvfs = CUR_VALID_FS;
		if( cvfs && strlen(cvfs) ) {
			if( SG_ERROR_NONE != ( errc = sg_update_string( &valid_file_systems[i], cvfs ) ) ) {
				CLEANUP_INIT_VALID_FS
				WARN_LOG_FMT("disk", "couldn't update index %d for list of valid file systems", i);
				RETURN_FROM_PREVIOUS_ERROR( "disk", errc );
			}
		}
	}

	CLEANUP_INIT_VALID_FS
#endif

	/* gaps will be sorted to end */
	qsort( valid_file_systems, num_valid_file_systems, sizeof(valid_file_systems[0]), cmp_valid_fs );

#if defined(CAN_SHRINK_VALID_FS)
	/* shrink list of valid fs to remove gaps in sys_fs_types list */
	for (i = 0; i < num_valid_file_systems; ++i) {
		if( NULL == valid_file_systems[i] ) {
			break;
		}
	}
#endif
#if defined(CAN_SHRINK_VALID_FS) || defined(LINUX)
	if( i < num_valid_file_systems ) {
		char **tmp;
		num_valid_file_systems = i;
		tmp = sg_realloc( valid_file_systems, (num_valid_file_systems + 1) * sizeof(valid_file_systems[0]) );
		/* doesn't matter when it fails, there're only unused NULL's at the end */
		if( tmp )
			valid_file_systems = tmp;
	}
#endif

	return SG_ERROR_NONE;
}

const char **
sg_get_valid_filesystems(size_t *entries) {
	if( entries )
		(*entries) = num_valid_file_systems;
	return (const char **)valid_file_systems;
}

sg_error
sg_set_valid_filesystems(const char *valid_fs[]) {
	size_t num_new_valid_fs = 0, tmp;
	char **new_valid_fs = NULL;
	char ** old_valid_fs = valid_file_systems;
	size_t num_old_valid_fs = num_valid_file_systems;

	/* XXX this must be locked - from caller ? */
	if( valid_fs && *valid_fs ) {

		while( valid_fs[num_new_valid_fs++] ) {} /* Note: post-inc is intensional */

		new_valid_fs = calloc( num_new_valid_fs + 1, sizeof(char *) );
		if( 0 == new_valid_fs ) {
			RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_MALLOC, "set_valid_filesystems");
		}

		tmp = num_new_valid_fs;
		while( tmp-- > 0 ) {
			sg_error errc;
			if( SG_ERROR_NONE != ( errc = sg_update_string( &new_valid_fs[tmp], valid_fs[tmp] ) ) ) {
				WARN_LOG_FMT("disk", "couldn't update index %d for list of valid file systems", tmp);
				while( valid_fs[tmp] ) {
				       free(new_valid_fs[tmp]);
				       ++tmp;
				}
				RETURN_FROM_PREVIOUS_ERROR( "disk", errc );
			}
		}

		qsort( new_valid_fs, num_new_valid_fs, sizeof(new_valid_fs[0]), cmp_valid_fs );
	}

	valid_file_systems = new_valid_fs;
	num_valid_file_systems = num_new_valid_fs;

	if( old_valid_fs ) {
		size_t i;

		for( i = 0; i < num_old_valid_fs; ++i )
			free( old_valid_fs[i] );
		free( old_valid_fs );
	}

	return SG_ERROR_NONE;
}

static int
is_valid_fs_type(const char *type) {
	char *key = bsearch( &type, valid_file_systems, num_valid_file_systems, sizeof(valid_file_systems[0]), cmp_valid_fs );
	TRACE_LOG_FMT( "disk", "is_valid_fs_type(%s): %d", type, key != NULL );
	return key != NULL;
}

#ifdef WIN32
#define BUFSIZE MAX_PATH+1
struct sg_win32_drvent {
	UINT drive_type;
	TCHAR volume_name_buf[BUFSIZE];
	DWORD volume_serial;
	DWORD max_component_len;
	DWORD fs_flags;
	TCHAR filesys_name_buf[BUFSIZE];
};

static DWORD
getdrvent(LPTSTR vol, unsigned check_mask, struct sg_win32_drvent *buf) {
	char drive[4] = " :\\";

	assert(vol);
	assert(buf);

	if( isalpha(*vol) ) {
		*drive = *vol;
		vol = drive;
	}

	buf->drive_type = GetDriveType(vol);
	if( ( DRIVE_UNKNOWN == buf->drive_type ) || ( DRIVE_NO_ROOT_DIR == buf->drive_type ) )
		return 1;

	if( check_mask & ( 1 << buf->drive_type ) ) {
		return GetVolumeInformation( vol,
					     &buf->volume_name_buf, sizeof(buf->volume_name_buf),
					     &buf->volume_serial, &buf->max_component_len,
					     &buf->fs_flags,
					     &buf->filesys_name_buf, sizeof(buf->filesys_name_buf) );
	}

	return 1;
}
#endif

/*
 * setup code
 */

#define SG_FS_STAT_IDX		0
#define SG_DISK_IO_NOW_IDX	1
#define SG_DISK_IO_DIFF_IDX	2
#define SG_DISK_IDX_COUNT       3

EXTENDED_COMP_SETUP(disk,SG_DISK_IDX_COUNT,NULL);

#if 0
#ifdef LINUX
static regex_t not_part_re, part_re;
#endif
#endif

sg_error
sg_disk_init_comp(unsigned id){
	sg_error rc;
	GLOBAL_SET_ID(disk,id);

	if( SG_ERROR_NONE != ( rc = init_valid_fs_types() ) ){
		RETURN_FROM_PREVIOUS_ERROR( "disk", rc );
	}

#if 0
#ifdef LINUX
	if (regcomp(&part_re, "^(.*/)?[^/]*[0-9]$", REG_EXTENDED | REG_NOSUB) != 0) {
		RETURN_WITH_SET_ERROR("disk", SG_ERROR_PARSE, NULL); /* with regex errcode */
	}
	if (regcomp(&not_part_re, "^(.*/)?[^/0-9]+[0-9]+d[0-9]+$", REG_EXTENDED | REG_NOSUB) != 0) {
		RETURN_WITH_SET_ERROR("disk", SG_ERROR_PARSE, NULL);
	}
#endif
#endif

	return SG_ERROR_NONE;
}

void
sg_disk_destroy_comp(void)
{
#if 0
#ifdef LINUX
	regfree(&part_re);
	regfree(&not_part_re);
#endif
#endif
	if( valid_file_systems ) {
		size_t i;

		for( i = 0; i < num_valid_file_systems; ++i )
			free( valid_file_systems[i] );
		free( valid_file_systems );

		valid_file_systems = NULL;
	}

#if defined(AIX)
	if( sys_fs_types ) {
		size_t i;

		for( i = 0; i < num_sys_fs_types; ++i )
			free( valid_file_systems[i] );
		free( sys_fs_types );

		sys_fs_types = NULL;
	}
#endif
}

EASY_COMP_CLEANUP_FN(disk,SG_DISK_IDX_COUNT)

/*
 * real stuff
 */

static sg_error
sg_get_fs_list_int(sg_vector **fs_stats_vector_ptr) {
	size_t num_fs = 0;
	sg_fs_stats *fs_ptr;
	time_t now = time(NULL);

#if defined(HAVE_GETVFSSTAT)
	struct statvfs *mntbuf;
	int fs_count, i;
#define GETMOUNTS_FN getvfsstat
#elif defined(HAVE_GETFSSTAT)
	struct statfs *mntbuf;
	int fs_count, i;
#define GETMOUNTS_FN getfsstat
#if !defined(ST_NOWAIT) && defined(MNT_NOWAIT)
#define ST_NOWAIT MNT_NOWAIT
#endif
#elif defined(HAVE_MNTCTL)
	struct vmount *buf, *mp;
	size_t bufsize = 4096;
	ssize_t rc, i;
#elif defined(HAVE_GETMNTENT_R) || defined(HAVE_THREADSAFE_GETMNTENT)
	FILE *f;
# ifdef HAVE_GETMNTENT_R
	char buf[4096];
#  ifdef HAVE_STRUCT_MNTENT
	struct mntent mp;
#   ifdef GETMNTENT_R_RETURN_INT
	int rc;
#   else
	struct mntent *rc;
#   endif
#  endif
# elif defined(HAVE_THREADSAFE_GETMNTENT)
#  if defined(HAVE_STRUCT_MNTTAB)
	struct mnttab mp;
	int rc; /* XXX exists an struct mnttab *getmntent(FILE *f); ? */
#  elif defined(HAVE_STRUCT_MNTENT)
	struct mntent *mp, *rc;
#  endif
# endif
# ifndef MNT_MNTTAB
	static const char *mnttabs[] = { "/etc/mnttab", "/etc/mtab" };
	unsigned i;
# endif
#elif defined(WIN32)
	LPTSTR buf = NULL, p;
	DWORD dynbuflen;
	struct sg_win32_drvent mp;
	/* lp_buf[0]='\0'; */
#endif

	assert(fs_stats_vector_ptr);

#if defined(HAVE_GETVFSSTAT) || defined(HAVE_GETFSSTAT)
# define return_set_mnt_error(error, mount_fn) RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", error, #mount_fn)
# ifdef ST_NOWAIT
#  define GETMOUNTS_FLAG ST_NOWAIT
# elif defined(MNT_NOWAIT)
#  define GETMOUNTS_FLAG MNT_NOWAIT
# endif
	fs_count = GETMOUNTS_FN(NULL, 0, GETMOUNTS_FLAG);
	if( fs_count < 0 ) {
		return_set_mnt_error(SG_ERROR_GETMNTINFO, #GETMOUNTS_FN);
	}

	if( 0 == fs_count ) {
		VECTOR_UPDATE(fs_stats_vector_ptr, 0, fs_ptr, sg_fs_stats);
		return SG_ERROR_NONE;
	}

	mntbuf = malloc( sizeof(*mntbuf) * fs_count );
	if( NULL == mntbuf ) {
		return_set_mnt_error(SG_ERROR_MALLOC, #GETMOUNTS_FN);
	}

	if( fs_count != GETMOUNTS_FN(mntbuf, sizeof(*mntbuf) * fs_count, GETMOUNTS_FLAG) ) {
		free(mntbuf);
		return_set_mnt_error(SG_ERROR_GETMNTINFO, #GETMOUNTS_FN);
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(mntbuf);

	VECTOR_UPDATE(fs_stats_vector_ptr, fs_count, fs_ptr, sg_fs_stats);

#define SG_FS_LOOP_INIT i = 0
#define SG_FS_LOOP_COND i < fs_count
#define SG_FS_LOOP_ITER ++i

#define SG_FS_FSTYPENAME	mntbuf[i].f_fstypename
#define SG_FS_DEVNAME		mntbuf[i].f_mntfromname
#define SG_FS_MOUNTP		mntbuf[i].f_mntonname
#define SG_FS_DEVTYPE		sg_fs_unknown

#elif defined(HAVE_MNTCTL)
	if( NULL == sys_fs_types ) {
		RETURN_WITH_SET_ERROR( "disk", SG_ERROR_INITIALISATION, "sys_fs_types" );
	}
	buf = malloc( bufsize );
	if( NULL == buf ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_MALLOC, "mntctl");
	}

	rc = mntctl( MCTL_QUERY, bufsize, (char *)buf );
	if( 0 == rc ) {
		bufsize = buf->vmt_revision; /* AIX misuses this field as "required size" */
		void *newbuf = realloc( buf, bufsize );
		if( NULL == newbuf ) {
			free(buf);
			RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_MALLOC, "mntctl");
		}
		buf = newbuf;

		rc = mntctl( MCTL_QUERY, bufsize, (char *)buf );
	}

	if( -1 == rc ) {
		free(buf);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_SYSCTLBYNAME, "mntctl");
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(buf);

	VECTOR_UPDATE(fs_stats_vector_ptr, rc, fs_ptr, sg_fs_stats);

#define SG_FS_LOOP_INIT i = 0, mp = buf
#define SG_FS_LOOP_COND i < rc
#define SG_FS_LOOP_ITER ++i, mp = (struct vmount *)(((char *)mp) + mp->vmt_length )

#define SG_FS_FSTYPENAME	sys_fs_types[mp->vmt_gfstype]
#define SG_FS_DEVNAME		vmt2dataptr(mp,0)
#define SG_FS_MOUNTP		vmt2dataptr(mp,1)
#define SG_FS_DEVTYPE		sg_fs_unknown

#elif defined(HAVE_GETMNTENT_R) || defined(HAVE_THREADSAFE_GETMNTENT)
# ifdef MNT_MNTTAB
	f = setmntent(MNT_MNTTAB, "r");
# else
	for( i = 0; i < lengthof(mnttabs); ++i ) {
		if( NULL != ( f = setmntent(mnttabs[i], "r") ) )
			break;
	}
# endif
	if( NULL == f ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_SETMNTENT, NULL);
	}

#define NEED_VECTOR_UPDATE

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP endmntent(f);

# if defined(HAVE_GETMNTENT_R)
#  define SG_FS_LOOP_INIT rc = getmntent_r(f, &mp, buf, sizeof(buf))
# elif defined(HAVE_THREADSAFE_GETMNTENT)
#  ifdef HAVE_STRUCT_MNTTAB
#   define SG_FS_LOOP_INIT rc = getmntent(f, &mp)
#  else
#   define SG_FS_LOOP_INIT rc = mp = getmntent(f)
#  endif
# endif
# if defined(GETMNTENT_R_RETURN_INT) || defined(HAVE_STRUCT_MNTTAB)
#  define SG_FS_LOOP_COND rc == 0
# else
#  define SG_FS_LOOP_COND rc != NULL
# endif
#define SG_FS_LOOP_ITER SG_FS_LOOP_INIT

# ifdef HAVE_STRUCT_MNTENT
#  ifdef HAVE_GETMNTENT_R
#   define SG_FS_FSTYPENAME	mp.mnt_type
#   define SG_FS_DEVNAME	mp.mnt_fsname
#   define SG_FS_MOUNTP		mp.mnt_dir
#   define SG_FS_DEVTYPE	sg_fs_unknown
#  else
#   define SG_FS_FSTYPENAME	mp->mnt_type
#   define SG_FS_DEVNAME	mp->mnt_fsname
#   define SG_FS_MOUNTP		mp->mnt_dir
#   define SG_FS_DEVTYPE	sg_fs_unknown
#  endif
# elif defined(HAVE_STRUCT_MNTTAB)
#  define SG_FS_FSTYPENAME	mp.mnt_fstype
#  define SG_FS_DEVNAME		mp.mnt_special
#  define SG_FS_MOUNTP		mp.mnt_mountp
#  define SG_FS_DEVTYPE		sg_fs_unknown
# endif

#elif defined(WIN32)
	dynbuflen = GetLogicalDriveStrings(0, NULL);
	if( 0 == dynbuflen ) {
		RETURN_WITH_SET_ERROR("disk", SG_ERROR_GETMNTINFO, "GetLogicalDriveStrings");
	}

	buf = malloc(dynbuflen);
	if( NULL == buf ) {
		RETURN_WITH_SET_ERROR("disk", SG_ERROR_MALLOC, "GetLogicalDriveStrings");
	}

	dynbuflen = GetLogicalDriveStrings(dynbuflen, buf);
	if( 0 == dynbuflen ) {
		free(buf);
		RETURN_WITH_SET_ERROR("disk", SG_ERROR_GETMNTINFO, "GetLogicalDriveStrings");
	}

	if (!(GetLogicalDriveStrings(BUFSIZE-1, lp_buf))) {
		RETURN_WITH_SET_ERROR("disk", SG_ERROR_GETMNTINFO, "GetLogicalDriveStrings");
	}

	SetLastError(0);

#define NEED_VECTOR_UPDATE

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(buf);

#define SG_FS_LOOP_INIT p = buf, rc = getdrvent(p, (1 << DRIVE_FIXED)|(1<<DRIVE_RAMDISK), &mp )
#define SG_FS_LOOP_COND '\0' != *p
#define SG_FS_LOOP_ITER p += strlen(p) + 1, rc = getdrvent(p, (1 << DRIVE_FIXED)|(1<<DRIVE_RAMDISK), &mp )

#define SG_MP_FSTYPENAME	mp.filesys_name_buf
#define SG_MP_DEVNAME		p
#define SG_MP_MOUNTP		p
#define SG_FS_DEVTYPE		sg_fs_unknown

#else
	RETURN_WITH_SET_ERROR("disk", SG_ERROR_UNSUPPORTED, OS_TYPE);
#define UNSUPPORTED
#endif

#ifndef UNSUPPORTED

#ifndef WIN32
	errno = 0;
#endif

	for( SG_FS_LOOP_INIT; SG_FS_LOOP_COND; SG_FS_LOOP_ITER ) {

		sg_error upderr;

#ifdef WIN32
		if( !mp.volume_serial ) /* skip unformatted disks - XXX recheck this for remote drives ... */
			continue;
#endif

#ifdef NEED_VECTOR_UPDATE
		VECTOR_UPDATE(fs_stats_vector_ptr, num_fs + 1, fs_ptr, sg_fs_stats);
#endif

		if( ( ( upderr = sg_update_string( &fs_ptr[num_fs].device_name, SG_FS_DEVNAME ) ) != SG_ERROR_NONE ) ||
		    ( ( upderr = sg_update_string( &fs_ptr[num_fs].fs_type, SG_FS_FSTYPENAME ) ) != SG_ERROR_NONE ) ||
		    ( ( upderr = sg_update_string( &fs_ptr[num_fs].mnt_point, SG_FS_MOUNTP ) ) != SG_ERROR_NONE ) ) {
			RETURN_FROM_PREVIOUS_ERROR( "disk", upderr );
		}

		fs_ptr[num_fs].device_type = SG_FS_DEVTYPE;
		fs_ptr[num_fs].systime = now;

		++num_fs;
	}

	VECTOR_UPDATE_ERROR_CLEANUP

#ifdef WIN32
	if( GetLastError() != 0 ) {
		RETURN_WITH_SET_ERROR("disk", SG_ERROR_DISKINFO, "GetVolumeInformation"); /* XXX implement GetLastError() capability for set_error...*/
	}
#else
	if( errno != 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_DISKINFO, "getmntent");
	}
#endif

	return SG_ERROR_NONE;
#endif /* !UNSUPPORTED */
}

#undef UNSUPPORTED

static sg_error
sg_fill_fs_stat_int(sg_fs_stats *fs_stats_ptr) {
#if defined(HAVE_STATVFS64) && defined(HAVE_STRUCT_STATVFS64)
	struct statvfs64 fs;
#elif defined(HAVE_STATVFS) && defined(HAVE_STRUCT_STATVFS)
	struct statvfs fs;
#elif defined(HAVE_STATFS) && defined(HAVE_STRUCT_STATFS)
	struct statfs fs;
#elif defined(WIN32)
	DWORD sectors_per_cluster, bytes_per_sector, free_clusters, total_clusters;
#endif

#if defined(HAVE_STATVFS64) && defined(HAVE_STRUCT_STATVFS64)
	memset( &fs, 0, sizeof(fs) );
	if( statvfs64( fs_stats_ptr->mnt_point, &fs ) != 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO( "disk", SG_ERROR_DISKINFO,
						  "statvfs64 for mnt_point at %s (device: %s, fs_type: %s)",
						  fs_stats_ptr->mnt_point, fs_stats_ptr->device_name,
						  fs_stats_ptr->fs_type );
	}

	fs_stats_ptr->total_inodes = fs.f_files;
	fs_stats_ptr->free_inodes  = fs.f_ffree;
	fs_stats_ptr->avail_inodes = fs.f_favail;
	fs_stats_ptr->used_inodes  = fs_stats_ptr->total_inodes - fs_stats_ptr->free_inodes;

#if defined(HAVE_STATVFS_FIOSIZE)
	fs_stats_ptr->io_size      = fs.f_iosize;
#else
	fs_stats_ptr->io_size      = fs.f_bsize;
#endif
#if defined(HAVE_STATVFS_FFRSIZE)
	fs_stats_ptr->block_size   = fs.f_frsize;
#else
	fs_stats_ptr->block_size   = fs.f_bsize;
#endif

	fs_stats_ptr->total_blocks = fs.f_blocks;
	fs_stats_ptr->free_blocks  = fs.f_bfree;
	fs_stats_ptr->avail_blocks = fs.f_bavail;
	fs_stats_ptr->used_blocks  = fs_stats_ptr->total_blocks - fs_stats_ptr->free_blocks;

#elif defined(HAVE_STATVFS) && defined(HAVE_STRUCT_STATVFS)
	memset( &fs, 0, sizeof(fs) );
	if( statvfs( fs_stats_ptr->mnt_point, &fs ) != 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO( "disk", SG_ERROR_DISKINFO,
						  "statvfs for mnt_point at %s (device: %s, fs_type: %s)",
						  fs_stats_ptr->mnt_point, fs_stats_ptr->device_name,
						  fs_stats_ptr->fs_type );
	}

	fs_stats_ptr->total_inodes = fs.f_files;
	fs_stats_ptr->free_inodes  = fs.f_ffree;
	fs_stats_ptr->avail_inodes = fs.f_favail;
	fs_stats_ptr->used_inodes  = fs_stats_ptr->total_inodes - fs_stats_ptr->free_inodes;

#if defined(HAVE_STATVFS_FIOSIZE)
	fs_stats_ptr->io_size      = fs.f_iosize;
#else
	fs_stats_ptr->io_size      = fs.f_bsize;
#endif
#if defined(HAVE_STATVFS_FFRSIZE)
	fs_stats_ptr->block_size   = fs.f_frsize;
#else
	fs_stats_ptr->block_size   = fs.f_bsize;
#endif

	fs_stats_ptr->total_blocks = fs.f_blocks;
	fs_stats_ptr->free_blocks  = fs.f_bfree;
	fs_stats_ptr->avail_blocks = fs.f_bavail;
	fs_stats_ptr->used_blocks  = fs_stats_ptr->total_blocks - fs_stats_ptr->free_blocks;

#elif defined(HAVE_STATFS) && defined(HAVE_STRUCT_STATFS)
	memset( &fs, 0, sizeof(fs) );
	if( statfs( fs_stats_ptr->mnt_point, &fs ) != 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO( "disk", SG_ERROR_DISKINFO,
						  "statfs for mnt_point at %s (device: %s, fs_type: %s)",
						  fs_stats_ptr->mnt_point, fs_stats_ptr->device_name,
						  fs_stats_ptr->fs_type );
	}

	fs_stats_ptr->total_inodes = fs.f_files;
	fs_stats_ptr->free_inodes  = fs.f_ffree;
#ifdef HAVE_STATFS_FFAVAIL
	fs_stats_ptr->avail_inodes = fs.f_favail;
#else
	fs_stats_ptr->avail_inodes =
#endif
	fs_stats_ptr->used_inodes  = fs_stats_ptr->total_inodes - fs_stats_ptr->free_inodes;

#if defined(HAVE_STATFS_FIOSIZE)
	fs_stats_ptr->io_size      = fs.f_iosize;
#elif defined(HAVE_STATFS_FFRSIZE)
	fs_stats_ptr->io_size      = fs.f_frsize;
#else
	fs_stats_ptr->io_size      = fs.f_bsize;
#endif
	/* no system found which implements behavior like statvfs
	 * f_bsize vs. f_frsize but has no statvfs */
	fs_stats_ptr->block_size   = fs.f_bsize;

	fs_stats_ptr->total_blocks = fs.f_blocks;
	fs_stats_ptr->free_blocks  = fs.f_bfree;
	fs_stats_ptr->avail_blocks = fs.f_bavail;
	fs_stats_ptr->used_blocks  = fs_stats_ptr->total_blocks - fs_stats_ptr->free_blocks;
#elif defined(WIN32)
	if( GetDiskFreeSpace( fs_stats_ptr->mnt_point,
	    		      &sectors_per_cluster, &bytes_per_sector,
			      &free_clusters, &total_clusters ) == 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO( "disk", SG_ERROR_DISKINFO, "GetDiskFreeSpace for %s", fs_stats_ptr->mnt_point );
	}

	fs_stats_ptr->total_inodes = 0;
	fs_stats_ptr->free_inodes  = 0;
	fs_stats_ptr->avail_inodes = 0;
	fs_stats_ptr->used_inodes  = 0;

	fs_stats_ptr->io_size      =
	fs_stats_ptr->block_size   = ((long long)sectors_per_cluster) * bytes_per_sector;

	fs_stats_ptr->total_blocks = fs_stats_ptr->block_size * total_clusters;
	fs_stats_ptr->free_blocks  = (long long)free_clusters;
	fs_stats_ptr->avail_blocks =
	fs_stats_ptr->used_blocks  = fs_stats_ptr->total_block - fs_stats_ptr->free_blocks;
#else
	RETURN_WITH_SET_ERROR("disk", SG_ERROR_UNSUPPORTED, OS_TYPE);
#define UNSUPPORTED
#endif

#ifndef UNSUPPORTED
	fs_stats_ptr->size  = fs_stats_ptr->block_size * fs_stats_ptr->total_blocks;
	fs_stats_ptr->avail = fs_stats_ptr->block_size * fs_stats_ptr->avail_blocks;
	fs_stats_ptr->free = fs_stats_ptr->block_size * fs_stats_ptr->free_blocks;
	fs_stats_ptr->used = fs_stats_ptr->block_size * fs_stats_ptr->used_blocks;

	fs_stats_ptr->systime = time(NULL);

	return SG_ERROR_NONE;
#endif
}

static sg_error
sg_get_fs_stats_int(sg_vector **fs_stats_vector_ptr){
	sg_vector *tmp = NULL;
	sg_error err = sg_get_fs_list_int(&tmp);
	size_t i, j, n = 0;
	sg_fs_stats *items, *item = VECTOR_DATA(tmp);
	unsigned valid[(VECTOR_ITEM_COUNT(tmp) / (8 * sizeof(unsigned))) + 1];

	memset( valid, 0, sizeof(valid) );

	if( ( SG_ERROR_NONE == err ) && (NULL != tmp) ) {

		i = VECTOR_ITEM_COUNT(tmp);
		item += i;
		while( i > 0 ) {
			 --i; --item;
			if( is_valid_fs_type(item->fs_type) ) {
				if( SG_ERROR_NONE != sg_fill_fs_stat_int(item) )
					continue;
				BIT_SET(valid, i);
				++n;
			}
			else {
				/* kick'em out of the list :P */
			}
		}
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP

	VECTOR_UPDATE(fs_stats_vector_ptr, n, items, sg_fs_stats);

	/* both assignment should be useless, because they should already on these values */
	i = 0;
	item = VECTOR_DATA(tmp);
	for( j = 0; j < n; ++j, ++i ) {
		BIT_SCAN_FWD(valid, i);
		if( i >= VECTOR_ITEM_COUNT(tmp) ) /* shouldn't happen ?! */
			break;
		sg_fs_stats_item_copy(items+j, item+i);
	}

	/* XXX shouldn't fail !? */
	assert( j == n );

	sg_vector_free(tmp);

	return err;
}

MULTI_COMP_ACCESS(sg_get_fs_stats,disk,fs,SG_FS_STAT_IDX)

int
sg_fs_compare_device_name(const void *va, const void *vb) {
	const sg_fs_stats *a = va, *b = vb;
	return strcmp(a->device_name, b->device_name);
}

int
sg_fs_compare_mnt_point(const void *va, const void *vb) {
	const sg_fs_stats *a = va, *b = vb;
	return strcmp(a->mnt_point, b->mnt_point);
}

#if 0
#ifdef LINUX
typedef struct {
	int major;
	int minor;
} partition;
#endif
#endif

#ifdef HPUX
static const char *disk_devs_in[] = { "/dev/dsk", "/dev/disk" };
#endif

static sg_error
sg_get_disk_io_stats_int( sg_vector **disk_io_stats_vector_ptr ) {
	size_t num_diskio = 0;
	sg_disk_io_stats *disk_io_stats;

#ifdef HPUX
	long long rbytes = 0, wbytes = 0;
	struct stat lstatinfo;
#define DISK_BATCH 30
	struct pst_diskinfo pstat_diskinfo[DISK_BATCH];
	char fullpathbuf[1024] = {0};
	dev_t diskid;
	DIR *dh = NULL;
	int diskidx = 0;
	int num, i;
	size_t n;
	time_t now = time(NULL);
#elif defined(SOLARIS)
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_io_t kios;
	time_t now = time(NULL);
#elif defined(LINUX)
	FILE *f;
#define LINE_BUF_SIZE 256
	char line_buf[LINE_BUF_SIZE];
#if 0
	int has_pp_stats = 1;
#endif
	time_t now = time(NULL);
	const char *format;
#elif defined(HAVE_STRUCT_IO_SYSCTL) || defined(HAVE_STRUCT_DISKSTATS) || defined(HAVE_STRUCT_DISK_SYSCTL)
	int num_disks, i;
	size_t size;
#  if defined(HAVE_STRUCT_IO_SYSCTL)
	int mib[3] = { CTL_HW, HW_IOSTATS, sizeof(struct io_sysctl) };
#    define STATS_TYPE struct io_sysctl
#    define STATS_NAME(s) (s.name)
#    define STATS_RBYTES(s) (s.rbytes)
#    define STATS_WBYTES(s) (s.wbytes)
#  elif defined(HAVE_STRUCT_DISK_SYSCTL)
	int mib[3] = { CTL_HW, HW_DISKSTATS, sizeof(struct disk_sysctl) };
#    define STATS_TYPE struct disk_sysctl
#    define STATS_NAME(s) (s.dk_name)
#    if defined(HAVE_DK_RBYTES)
#      define STATS_RBYTES(s) (s.dk_rbytes)
#      define STATS_WBYTES(s) (s.dk_wbytes)
#    else
#      define STATS_RBYTES(s) (s.dk_bytes)
#      define STATS_WBYTES(s) (s.dk_bytes)
#    endif
#  else
	int mib[2] = { CTL_HW, HW_DISKSTATS };
#    define STATS_TYPE struct diskstats
#    define STATS_NAME(s) (s.ds_name)
#    if defined(HAVE_DISKSTAT_DS_RBYTES)
#      define STATS_RBYTES(s) (s.ds_rbytes)
#      define STATS_WBYTES(s) (s.ds_wbytes)
#    else
#      define STATS_RBYTES(s) (s.ds_bytes)
#      define STATS_WBYTES(s) (s.ds_bytes)
#    endif
#  endif
	STATS_TYPE *stats;
	time_t now = time(NULL);
#elif defined(HAVE_STRUCT_DEVSTAT) && defined(HAVE_STRUCT_STATINFO)
	struct statinfo stats;
	int counter;
#if 0
	struct device_selection *dev_sel = NULL;
	int n_selected, n_selections;
	long sel_gen;
#endif
	struct devstat *dev_ptr;
	time_t now = time(NULL);
#elif defined(AIX)
	ssize_t ret, disks;
	perfstat_disk_t *dskperf;
	perfstat_id_t name;
	time_t now = time(NULL);
#elif defined(WIN32)
	char *name;
	long long rbytes;
	long long wbytes;
#endif

	assert(disk_io_stats_vector_ptr);

	if( NULL == disk_io_stats_vector_ptr || NULL == *disk_io_stats_vector_ptr ) {
		RETURN_WITH_SET_ERROR("disk", SG_ERROR_INVALID_ARGUMENT, "disk_io_stats_vector_ptr");
	}

#ifdef AIX
#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(dskperf);

	/* check how many perfstat_disk_t structures are available */
	disks = perfstat_disk(NULL, NULL, sizeof(perfstat_disk_t), 0);
	if( disks == -1 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_PSTAT, "perfstat_disk(NULL)");
	}

	dskperf = malloc( sizeof(perfstat_disk_t) * disks );
	if( 0 == dskperf ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_MALLOC, "sg_get_disk_io_stats");
	}

	name.name[0] = 0;
	ret = perfstat_disk(&name, dskperf, sizeof(perfstat_disk_t), disks);
	if( ret == -1 ) {
		free(dskperf);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_PSTAT, "perfstat_disk");
	}

	VECTOR_UPDATE(disk_io_stats_vector_ptr, (size_t)ret, disk_io_stats, sg_disk_io_stats);

	for( num_diskio = 0; num_diskio < (size_t)ret; ++num_diskio ) {

		disk_io_stats[num_diskio].read_bytes = dskperf[num_diskio].bsize * dskperf[num_diskio].rblks;
		disk_io_stats[num_diskio].write_bytes = dskperf[num_diskio].bsize * dskperf[num_diskio].wblks;
		disk_io_stats[num_diskio].systime = now;

		if( disk_io_stats[num_diskio].disk_name == NULL ) {
			int i;
			for( i = 0; i < IDENTIFIER_LENGTH; ++i ) {
				char *s = dskperf[num_diskio].name + i;
				if( !(isalpha(*s) || isdigit(*s) ||
				      *s == '-' || *s == '_' || *s == ' ') ) {
					*s = 0;
					break;
				}
			}

			if( sg_update_string(&disk_io_stats[num_diskio].disk_name, dskperf[num_diskio].name) != SG_ERROR_NONE ) {
				VECTOR_UPDATE_ERROR_CLEANUP;
				RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
			}
		}
	}

	free(dskperf);
#elif defined(HPUX)
	for(;;) {
		size_t enabled_disks = 0;
		num = pstat_getdisk(pstat_diskinfo, sizeof pstat_diskinfo[0], DISK_BATCH, diskidx);
		if( num == -1 ) {
			RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_PSTAT, "pstat_getdisk(idx=%d)", diskidx);
		}
		else if (num == 0 ) {
			break;
		}

		for( i = 0; i < num; ++i ) {
			struct pst_diskinfo *di = &pstat_diskinfo[i];

			/* Skip "disabled" disks. */
			if( di->psd_status == 0 ) {
				continue;
			}

			++enabled_disks;
		}

		VECTOR_UPDATE(disk_io_stats_vector_ptr, num_diskio + enabled_disks, disk_io_stats, sg_disk_io_stats);

		for( i = 0; i < num; ++i ) {
			struct pst_diskinfo *di = &pstat_diskinfo[i];

			/* Skip "disabled" disks. */
			if( di->psd_status == 0 ) {
				continue;
			}

#ifdef HAVE_PST_DISKINFO_PSD_DKBYTEWRITE
			rbytes = di->psd_dkbyteread;
			wbytes = di->psd_dkbytewrite;
#else
			/* We can't seperate the reads from the writes, we'll
			 * just give the same to each. (This value is in
			 * 64-byte chunks according to the pstat header file,
			 * and can wrap to be negative.)
			 */
			rbytes = wbytes = ((long long) di->psd_dkwds) * 64LL;
#endif

			disk_io_stats[num_diskio].read_bytes = rbytes;
			disk_io_stats[num_diskio].write_bytes = wbytes;

			disk_io_stats[num_diskio].systime = now;

			/* FIXME This should use a static cache, like the Linux
			 * code below. */
			diskid = (di->psd_dev.psd_major << 24) | di->psd_dev.psd_minor;
			for( n = 0; n < lengthof(disk_devs_in); ++n ) {
				struct dirent dinfo, *result = NULL;
				dh = opendir(disk_devs_in[n]);
				if( dh == NULL ) {
					continue;
				}

				while( ( 0 == (readdir_r(dh, &dinfo, &result)) ) && ( result != NULL ) ) {
					snprintf(fullpathbuf, sizeof(fullpathbuf), "%s/%s", disk_devs_in[n], dinfo.d_name);
					if( lstat(fullpathbuf, &lstatinfo) < 0 ) {
						continue;
					}

					if( lstatinfo.st_rdev == diskid ) {
						if( sg_update_string(&disk_io_stats[num_diskio].disk_name, dinfo.d_name) != SG_ERROR_NONE ) {
							RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
						}
						break;
					}
				}
				closedir(dh);
			}

			if( disk_io_stats[num_diskio].disk_name == NULL ) {
				if( sg_update_string(&disk_io_stats[num_diskio].disk_name, di->psd_hw_path.psh_name) != SG_ERROR_NONE ) {
					RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
				}
			}

			++num_diskio;
		}
		diskidx = pstat_diskinfo[num - 1].psd_idx + 1;
	}
#elif defined(HAVE_STRUCT_IO_SYSCTL) || defined(HAVE_STRUCT_DISKSTATS) || defined(HAVE_STRUCT_DISK_SYSCTL)

	if( sysctl(mib, lengthof(mib), NULL, &size, NULL, 0) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_SYSCTL, "CTL_HW.HW_DISKSTATS");
	}

	num_disks = size / sizeof(STATS_TYPE);
	stats = sg_malloc(size);
	if( stats == NULL ) {
		RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(stats);

	if( sysctl(mib, lengthof(mib), stats, &size, NULL, 0) < 0 ) {
		VECTOR_UPDATE_ERROR_CLEANUP;
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_SYSCTL, "CTL_HW.HW_DISKSTATS");
	}

	VECTOR_UPDATE(disk_io_stats_vector_ptr, num_disks, disk_io_stats, sg_disk_io_stats);

	for( i = 0; i < num_disks; ++i ) {
#if 0
		/* Don't keep stats for disks that have never been used. FIXME: Why? */
		if (rbytes == 0 && wbytes == 0) {
			continue;
		}
#endif

		disk_io_stats[num_diskio].read_bytes = STATS_RBYTES(stats[i]);
		disk_io_stats[num_diskio].write_bytes = STATS_WBYTES(stats[i]);

		if( sg_update_string(&disk_io_stats[num_diskio].disk_name, STATS_NAME(stats[i])) != SG_ERROR_NONE ) {
			VECTOR_UPDATE_ERROR_CLEANUP;
			RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
		}
		disk_io_stats[num_diskio].systime = now;

		++num_diskio;
	}

	free(stats);
#elif defined(HAVE_STRUCT_DEVSTAT) && defined(HAVE_STRUCT_STATINFO)
	stats.dinfo = sg_malloc(sizeof(struct devinfo));
	if( stats.dinfo == NULL ) {
		RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
	}
	bzero( stats.dinfo, sizeof(struct devinfo) );
# if HAVE_DEVSTAT_GETDEVS
#  define GETDEVS_FN(target) devstat_getdevs(NULL, target)
#  define GETDEVS_FN_NAME "devstat_getdevs"
# else
#  define GETDEVS_FN(target) getdevs(target)
#  define GETDEVS_FN_NAME "getdevs"
# endif

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(stats.dinfo->mem_ptr); free(stats.dinfo);

	if( GETDEVS_FN( &stats) < 0 ) {
		VECTOR_UPDATE_ERROR_CLEANUP;
		RETURN_WITH_SET_ERROR("disk", SG_ERROR_DEVSTAT_GETDEVS, GETDEVS_FN_NAME );
	}

	for( counter = 0; counter < stats.dinfo->numdevs; ++counter ) {
		dev_ptr = &stats.dinfo->devices[counter];

		/* care only for disk-like block-devices */
		if( ( DEVSTAT_TYPE_DIRECT != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_SEQUENTIAL != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_WORM != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_CDROM != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_OPTICAL != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_CHANGER != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_STORARRAY != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_FLOPPY != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_PASS != dev_ptr->device_type ) )
			continue;

		++num_diskio;
	}

	VECTOR_UPDATE(disk_io_stats_vector_ptr, num_diskio, disk_io_stats, sg_disk_io_stats);

	for( num_diskio = 0, counter = 0; counter < stats.dinfo->numdevs; ++counter ) {
		dev_ptr = &stats.dinfo->devices[counter];

		/* care only for disk-like block-devices */
		if( ( DEVSTAT_TYPE_DIRECT != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_SEQUENTIAL != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_WORM != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_CDROM != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_OPTICAL != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_CHANGER != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_STORARRAY != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_FLOPPY != dev_ptr->device_type ) &&
		    ( DEVSTAT_TYPE_PASS != dev_ptr->device_type ) )
		    continue;

#if defined(HAVE_DEVSTAT_BYTES)
		disk_io_stats[num_diskio].read_bytes=dev_ptr->bytes[DEVSTAT_READ];
		disk_io_stats[num_diskio].write_bytes=dev_ptr->bytes[DEVSTAT_WRITE];
#elif defined(HAVE_DEVSTAT_BYTES_READ)
		disk_io_stats[num_diskio].read_bytes=dev_ptr->bytes_read;
		disk_io_stats[num_diskio].write_bytes=dev_ptr->bytes_written;
#endif
		if( disk_io_stats[num_diskio].disk_name != NULL ) {
			free(disk_io_stats[num_diskio].disk_name);
			disk_io_stats[num_diskio].disk_name = 0;
		}
		if( asprintf( &disk_io_stats[num_diskio].disk_name, "%s%d", dev_ptr->device_name, dev_ptr->unit_number ) == -1 ) {
			VECTOR_UPDATE_ERROR_CLEANUP;
			RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_ASPRINTF, NULL);
		}
		disk_io_stats[num_diskio].systime = now;

		++num_diskio;
	}

	free(stats.dinfo->mem_ptr); free(stats.dinfo);

#elif defined(SOLARIS)
	if( (kc = kstat_open()) == NULL ) {
		RETURN_WITH_SET_ERROR("disk", SG_ERROR_KSTAT_OPEN, NULL);
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP kstat_close(kc);

	for( ksp = kc->kc_chain; ksp; ksp = ksp->ks_next ) {
		if( !strcmp(ksp->ks_class, "disk") ) {

			if( ksp->ks_type != KSTAT_TYPE_IO )
				continue;
			/* We dont want metadevices appearins as num_diskio */
			if( strcmp( ksp->ks_module, "md" ) == 0 )
				continue;
			if( ( kstat_read(kc, ksp, &kios) ) == -1 ) {
				/* XXX intentional? */
				kstat_close(kc);
				RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_KSTAT_READ, NULL);
			}

			VECTOR_UPDATE(disk_io_stats_vector_ptr, num_diskio + 1, disk_io_stats, sg_disk_io_stats);

			disk_io_stats[num_diskio].read_bytes = kios.nread;
			disk_io_stats[num_diskio].write_bytes = kios.nwritten;
			if( sg_update_string(&disk_io_stats[num_diskio].disk_name,
					     sg_get_svr_from_bsd(ksp->ks_name)) != SG_ERROR_NONE ) {
				kstat_close(kc);
				RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
			}
			disk_io_stats[num_diskio].systime = now;

			++num_diskio;
		}
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP

	kstat_close(kc);
#elif defined(LINUX)
	num_diskio = 0;

	/* Read /proc/partitions to find what devices exist. Recent 2.4 kernels
	   have statistics in here too, so we can use those directly.
	   2.6 kernels have /proc/diskstats instead with almost (but not quite)
	   the same format. */

	f = fopen("/proc/diskstats", "r");
	format = " %*d %*d %99s %*d %*d %llu %*d %*d %*d %llu";
#if 0
	if (f == NULL) {
		f = fopen("/proc/partitions", "r");
		format = " %d %d %*d %99s %*d %*d %lld %*d %*d %*d %lld";
	}
#endif
	if( f == NULL ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("disk", SG_ERROR_OPEN, "/proc/diskstats");
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP fclose(f);

	while( fgets(line_buf, sizeof(line_buf), f) != NULL ) {
		char name[100];
		unsigned long long rsect, wsect;

		int nr = sscanf(line_buf, format, name, &rsect, &wsect);
		if( nr != 3 )
			continue;

#if 0
		/* Skip device names ending in numbers, since they're
		   partitions, unless they match the c0d0 pattern that some
		   RAID devices use. */
		/* FIXME: For 2.6+, we should probably be using sysfs to detect
		   this... */
		/* loopN, dm-NN, ramNm ... shouldn't be skipped,
		   neither partitions should in times of soft-raid via lvm
		   or varying i/o load on partitions */
		if ((regexec(&part_re, name, 0, NULL, 0) == 0) &&
		    (regexec(&not_part_re, name, 0, NULL, 0) != 0)) {
			continue;
		}
#endif

#if 0
		/* Linux 2.4 is unsupported until someone ports back ...*/
		if (nr < 5) {
			has_pp_stats = 0;
			rsect = 0;
			wsect = 0;
		}
#endif

		VECTOR_UPDATE(disk_io_stats_vector_ptr, num_diskio + 1, disk_io_stats, sg_disk_io_stats);

		if( sg_update_string(&disk_io_stats[num_diskio].disk_name, name) != SG_ERROR_NONE ) {
			fclose(f);
			RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
		}
		disk_io_stats[num_diskio].read_bytes = rsect * 512;
		disk_io_stats[num_diskio].write_bytes = wsect * 512;
		disk_io_stats[num_diskio].systime = now;
#if 0
		parts[n].major = major;
		parts[n].minor = minor;
#endif

		++num_diskio;
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP

	fclose(f);
#if 0
	f = NULL;

	if (!has_pp_stats) {
		/* This is an older kernel where /proc/partitions doesn't
		   contain stats. Read what we can from /proc/stat instead, and
		   fill in the appropriate bits of the list allocated above. */

		f = fopen("/proc/stat", "r");
		if (f == NULL) goto out;
		now = time(NULL);

		line_ptr = sg_f_read_line(f, "disk_io:");
		if (line_ptr == NULL) goto out;

		while((line_ptr=strchr(line_ptr, ' '))!=NULL){
			long long rsect, wsect;

			if (*++line_ptr == '\0') break;

			if((sscanf(line_ptr,
				"(%d,%d):(%*d, %*d, %lld, %*d, %lld)",
				&major, &minor, &rsect, &wsect)) != 4) {
					continue;
			}

			/* Find the corresponding device from earlier.
			   Just to add to the fun, "minor" is actually the disk
			   number, not the device minor, so we need to figure
			   out the real minor number based on the major!
			   This list is not exhaustive; if you're running
			   an older kernel you probably don't have fancy
			   I2O hardware anyway... */
			switch (major) {
			case 3:
			case 21:
			case 22:
			case 33:
			case 34:
			case 36:
			case 56:
			case 57:
			case 88:
			case 89:
			case 90:
			case 91:
				minor *= 64;
				break;
			case 9:
			case 43:
				break;
			default:
				minor *= 16;
				break;
			}
			for (i = 0; i < n; i++) {
				if (major == parts[i].major
					&& minor == parts[i].minor)
					break;
			}
			if (i == n) continue;

			/* We read the number of blocks. Blocks are stored in
			   512 bytes */
			disk_io_stats[i].read_bytes = rsect * 512;
			disk_io_stats[i].write_bytes = wsect * 512;
			disk_io_stats[i].systime = now;
		}
	}
#endif

#elif defined(WIN32)
	sg_clear_error();

	while( ( name = get_diskio(num_diskio, &rbytes, &wbytes) ) != NULL ) {
		VECTOR_UPDATE(disk_io_stats_vector_ptr, num_diskio + 1, disk_io_stats, sg_disk_io_stats);

		if( sg_update_string(&disk_io_stats[num_diskio].disk_name, name) != SG_ERROR_NONE ) {
			RETURN_FROM_PREVIOUS_ERROR( "disk", sg_get_error() );
		}
		sg_update_string(&name, NULL);
		disk_io_stats[num_diskio].read_bytes = rbytes;
		disk_io_stats[num_diskio].write_bytes = wbytes;

		disk_io_stats[num_diskio].systime = 0;

		++num_diskio;
	}
#else
	RETURN_WITH_SET_ERROR("disk", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif

	return SG_ERROR_NONE;
}

MULTI_COMP_ACCESS(sg_get_disk_io_stats,disk,disk_io,SG_DISK_IO_NOW_IDX)
MULTI_COMP_DIFF(sg_get_disk_io_stats_diff,sg_get_disk_io_stats,disk,disk_io,SG_DISK_IO_DIFF_IDX,SG_DISK_IO_NOW_IDX)

int
sg_disk_io_compare_name(const void *va, const void *vb) {
	const sg_disk_io_stats *a = va, *b = vb;
	return strcmp(a->disk_name, b->disk_name);
}

int
sg_disk_io_compare_traffic(const void *va, const void *vb) {
	const sg_disk_io_stats *a = va, *b = vb;
	unsigned long long tot_a = a->read_bytes + a->write_bytes;
	unsigned long long tot_b = b->read_bytes + b->write_bytes;
	return (tot_a == tot_b) ? 0 : (tot_a > tot_b) ? -1 : 1; /* XXX reverse order! */
}
