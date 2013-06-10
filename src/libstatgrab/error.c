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
#include "globals.h"
#if defined(HAVE_VSNPRINTF)
# include <stdarg.h>
#endif

#define ERROR_ARG_MAX 4096
struct sg_error_glob {
	sg_error error;
	int errno_value;
	char error_arg[ERROR_ARG_MAX];
#if !defined(HAVE_VSNPRINTF)
#define ERROR_OVERRIDE_PROTECTION MAGIC_EYE('e','r','r','e')
	unsigned error_override_protection;
#endif
};

#if !defined(HAVE_VSNPRINTF)
static struct sg_error_glob pre_init_errs = { SG_ERROR_NONE, 0, { '\0' }, ERROR_OVERRIDE_PROTECTION };
#else
static struct sg_error_glob pre_init_errs = { SG_ERROR_NONE, 0, { '\0' } };
#endif

DEFAULT_INIT_COMP(error,NULL);

static va_list empty_ap;

static sg_error
sg_error_init_comp(unsigned id) {
	struct sg_error_glob *error_glob = sg_comp_get_tls(id);
	GLOBAL_SET_ID(error,id);
	/* remember the initial errors ... */
	if( error_glob && ( pre_init_errs.error != SG_ERROR_NONE ) ) {
		*error_glob = pre_init_errs;
	}
	/* init empty va_list */
	memset( &empty_ap, 0, sizeof(empty_ap) );
	return SG_ERROR_NONE;
}

EASY_COMP_DESTROY_FN(error)
static void sg_error_cleanup_comp(void *p)
{
	(void)p;
	TRACE_LOG_FMT("error", "sg_error_cleanup_comp(%p)", p);
}

static struct sg_error_glob *
sg_get_error_glob(void) {
	struct sg_error_glob *error_glob = GLOBAL_GET_TLS(error);
	if( !error_glob )
		error_glob = &pre_init_errs;
	return error_glob;
}

static sg_error
sg_set_error_int(sg_error code, int errno_value, const char *arg, va_list ap) {
	struct sg_error_glob *error_glob = sg_get_error_glob();

	error_glob->errno_value = errno_value;
	error_glob->error = code;
	if (arg != NULL) {
#if defined(HAVE_VSNPRINTF)
		vsnprintf( error_glob->error_arg, sizeof(error_glob->error_arg), arg, ap );
#else
		error_glob->error_override_protection = ERROR_OVERRIDE_PROTECTION;
		vsprintf( error_glob->error_arg, arg, ap );
		assert( error_glob->error_override_protection == ERROR_OVERRIDE_PROTECTION );
#endif
	}
	else {
		/* FIXME is this the best idea? */
		error_glob->error_arg[0] = '\0';
	}

	return code;
}

__sg_private void
sg_clear_error(void) {
	struct sg_error_glob *error_glob = sg_get_error_glob();
	if( error_glob ) {
		error_glob->error = SG_ERROR_NONE;
		error_glob->errno_value = 0;
		error_glob->error_arg[0] = '\0';
	}
}

sg_error
sg_set_error_fmt(sg_error code, const char *arg, ...) {
	va_list ap;
	sg_error rc;
	va_start(ap, arg);
	rc = sg_set_error_int(code, 0, arg, ap);
	va_end(ap);
	return rc;
}

sg_error
sg_set_error_with_errno_fmt(sg_error code, const char *arg, ...) {
	va_list ap;
	sg_error rc;
	va_start(ap, arg);
	rc = sg_set_error_int(code, errno, arg, ap);
	va_end(ap);
	return rc;
}

sg_error
sg_set_error_with_errno_code_fmt(sg_error code, int errno_value, const char *arg, ...) {
	va_list ap;
	sg_error rc;
	va_start(ap, arg);
	rc = sg_set_error_int(code, errno_value, arg, ap);
	va_end(ap);
	return rc;
}

sg_error
sg_get_error(void) {
	struct sg_error_glob *error_glob = sg_get_error_glob();
	if( !error_glob )
		return SG_ERROR_MEMSTATUS; /* In this case we can't set our global last error :-( */
	return error_glob->error;
}

const char *
sg_get_error_arg(void) {
	struct sg_error_glob *error_glob = sg_get_error_glob();
	if( !error_glob )
		return NULL; /* In this case we can't set our global last error :-( */
	return error_glob->error_arg;
}

int
sg_get_error_errno(void) {
	struct sg_error_glob *error_glob = sg_get_error_glob();
	if( !error_glob )
		return EAGAIN; /* In this case we can't set our global last error :-( */
	return error_glob->errno_value;
}

sg_error
sg_get_error_details(sg_error_details *err_details){
	struct sg_error_glob *error_glob = sg_get_error_glob();
	if(NULL == err_details) {
		return sg_set_error_int(SG_ERROR_INVALID_ARGUMENT, 0, "sg_get_error_details", empty_ap);
	}
	if( !error_glob ) {
		err_details->error = SG_ERROR_MEMSTATUS;
		err_details->errno_value = EINVAL;
		err_details->error_arg = "Can't get tls buffer";

		return SG_ERROR_MEMSTATUS; /* In this case we can't set our global last error :-( */
	}

	err_details->error = error_glob->error;
	err_details->errno_value = error_glob->errno_value;
	err_details->error_arg = error_glob->error_arg;

	return SG_ERROR_NONE;
}

const char *
sg_str_error(sg_error code) {
	switch (code) {
	case SG_ERROR_NONE:
		return "no error";
	case SG_ERROR_INVALID_ARGUMENT:
		return "invalid argument";
	case SG_ERROR_ASPRINTF:
		return "asprintf failed";
	case SG_ERROR_SPRINTF:
		return "sprintf failed";
	case SG_ERROR_DEVSTAT_GETDEVS:
		return "devstat_getdevs failed";
	case SG_ERROR_DEVSTAT_SELECTDEVS:
		return "devstat_selectdevs failed";
	case SG_ERROR_ENOENT:
		return "system call received ENOENT";
	case SG_ERROR_GETIFADDRS:
		return "getifaddress failed";
	case SG_ERROR_GETMNTINFO:
		return "getmntinfo failed";
	case SG_ERROR_GETPAGESIZE:
		return "getpagesize failed";
	case SG_ERROR_HOST:
		return "gather host information faile";
	case SG_ERROR_KSTAT_DATA_LOOKUP:
		return "kstat_data_lookup failed";
	case SG_ERROR_KSTAT_LOOKUP:
		return "kstat_lookup failed";
	case SG_ERROR_KSTAT_OPEN:
		return "kstat_open failed";
	case SG_ERROR_KSTAT_READ:
		return "kstat_read failed";
	case SG_ERROR_KVM_GETSWAPINFO:
		return "kvm_getswapinfo failed";
	case SG_ERROR_KVM_OPENFILES:
		return "kvm_openfiles failed";
	case SG_ERROR_MALLOC:
		return "malloc failed";
	case SG_ERROR_OPEN:
		return "failed to open file";
	case SG_ERROR_OPENDIR:
		return "failed to open directory";
	case SG_ERROR_READDIR:
		return "failed to read directory";
	case SG_ERROR_PARSE:
		return "failed to parse input";
	case SG_ERROR_SETEGID:
		return "setegid failed";
	case SG_ERROR_SETEUID:
		return "seteuid failed";
	case SG_ERROR_SETMNTENT:
		return "setmntent failed";
	case SG_ERROR_SOCKET:
		return "socket failed";
	case SG_ERROR_SWAPCTL:
		return "swapctl failed";
	case SG_ERROR_SYSINFO:
		return "sysinfo failed";
	case SG_ERROR_SYSCONF:
		return "sysconf failed";
	case SG_ERROR_SYSCTL:
		return "sysctl failed";
	case SG_ERROR_SYSCTLBYNAME:
		return "sysctlbyname failed";
	case SG_ERROR_SYSCTLNAMETOMIB:
		return "sysctlnametomib failed";
	case SG_ERROR_MACHCALL:
		return "mach kernel operation failed";
	case SG_ERROR_UNAME:
		return "uname failed";
	case SG_ERROR_UNSUPPORTED:
		return "unsupported function";
	case SG_ERROR_XSW_VER_MISMATCH:
		return "XSW version mismatch";
	case SG_ERROR_PSTAT:
		return "pstat failed";
	case SG_ERROR_PDHOPEN:
		return "PDH open failed";
	case SG_ERROR_PDHCOLLECT:
		return "PDH snapshot failed";
	case SG_ERROR_PDHADD:
		return "PDH add counter failed";
	case SG_ERROR_PDHREAD:
		return "PDH read counter failed";
	case SG_ERROR_DEVICES:
		return "failed to get device list";
	case SG_ERROR_PERMISSION:
		return "access violation";
	case SG_ERROR_DISKINFO:
		return "disk function failed";
	case SG_ERROR_MEMSTATUS:
		return "memory status failed";
	case SG_ERROR_GETMSG:
		return "getmsg failed";
	case SG_ERROR_PUTMSG:
		return "putmsg failed";
	case SG_ERROR_INITIALISATION:
		return "initialization error";
	case SG_ERROR_MUTEX_LOCK:
		return "lock mutex failed";
	case SG_ERROR_MUTEX_UNLOCK:
		return "unlock mutex failed";
	}
	return "unknown error";
}

#define SG_STRPERROR_BUF_SIZE 1024

char *
sg_strperror(char **buf, const sg_error_details * const err_details) {

#ifdef HAVE_STRERROR_R
	char errno_buf[128] = { '\0' };
#endif
	char *errno_msg = NULL;
	sg_error_details err_det;

	if((NULL == buf) || (NULL != *buf)) {
		sg_set_error_int(SG_ERROR_INVALID_ARGUMENT, 0, "strperror", empty_ap);
		return NULL;
	}

	if(NULL == err_details) {
		sg_get_error_details(&err_det);
	}
	else {
		err_det = *err_details;
	}

	if( NULL != (*buf = malloc(SG_STRPERROR_BUF_SIZE)) ) {
		if(err_det.errno_value) {
#ifdef HAVE_STRERROR_R
# ifdef STRERROR_R_RETURN_INT
			int rc;
# else
			char *rc;
# endif
			rc = strerror_r(err_det.errno_value, errno_buf, sizeof(errno_buf));
# ifdef STRERROR_R_RETURN_INT
			if(0 != rc)
# else
			if(NULL == rc)
# endif
			{
				sg_set_error_int(SG_ERROR_MALLOC, errno, "strerror_r", empty_ap);
				free(*buf);
				*buf = NULL;
				return NULL;
			}
			else {
# ifdef STRERROR_R_RETURN_INT
				errno_msg = errno_buf;
# else
				errno_msg = rc;
# endif
			}
#else
			errno_msg = strerror(errno);
#endif
		}

		snprintf( *buf, SG_STRPERROR_BUF_SIZE, "%s (%s%s%s)",
			  sg_str_error(err_det.error),
			  err_det.errno_value ? errno_msg : "",
			  err_det.errno_value ? ": " : "",
			  err_det.error_arg ? err_det.error_arg : "" );

	}
	else {
		sg_set_error_int(SG_ERROR_MALLOC, 0, "sg_strperror", empty_ap);
	}

	return *buf;
}
