/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2013 i-scream
 * Copyright (C) 2010-2013 Jens Rehsack
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

#include "helpers.h"
#include <tools.h>
#include <statgrab.h>
#include <poll.h>
#ifdef WITH_LIBLOG4CPLUS
#include <libgen.h>
#endif
#include <stdarg.h>

int *sig_flags[NSIG];

static void
sig_flagger(int signo) {
	if( sig_flags[signo] != NULL )
		++(*sig_flags[signo]);
}

int
register_sig_flagger(int signo, int *flag_ptr)
{
	if( ( sig_flags[signo] != NULL ) && (flag_ptr != NULL) )
		return EINVAL;

	if(flag_ptr) {
		signal( signo, NULL );
		sig_flags[signo] = NULL;
	}
	else {
		sig_flags[signo] = flag_ptr;
		signal( signo, sig_flagger );
	}

	return 0;
}

int
sg_warn(const char *prefix)
{
	sg_error_details err_det;
	char *errmsg = NULL;
	int rc;
	sg_error errc;

	if( SG_ERROR_NONE != ( errc = sg_get_error_details(&err_det) ) ) {
		fprintf(stderr, "can't get error details (%d, %s)\n", errc, sg_str_error(errc));
		return 0;
	}

	if( NULL == sg_strperror(&errmsg, &err_det) ) {
		errc = sg_get_error();
		fprintf(stderr, "panic: can't prepare error message (%d, %s)\n", errc, sg_str_error(errc));
		return 0;
	}

	rc = fprintf( stderr, "%s: %s\n", prefix, errmsg );

	free( errmsg );

	return rc;
}

void
sg_die(const char *prefix, int exit_code)
{
	sg_error_details err_det;
	char *errmsg = NULL;
	sg_error errc;

	if( SG_ERROR_NONE != ( errc = sg_get_error_details(&err_det) ) ) {
		fprintf(stderr, "panic: can't get error details (%d, %s)\n", errc, sg_str_error(errc));
		exit(exit_code);
	}

	if( NULL == sg_strperror(&errmsg, &err_det) ) {
		errc = sg_get_error();
		fprintf(stderr, "panic: can't prepare error message (%d, %s)\n", errc, sg_str_error(errc));
		exit(exit_code);
	}

	fprintf( stderr, "%s: %s\n", prefix, errmsg );

	free( errmsg );

	exit(exit_code);
}

static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char buf[4096];
	int printed;
	printed = vsnprintf( buf, sizeof(buf), fmt, ap);
	if(errnoflag)
		snprintf(buf + printed, sizeof(buf) - printed, ": %s", strerror(error) );
	fflush(stderr);
	fputs(buf, stderr);
	fputs("\n", stderr);
	fflush(NULL);
}

void
die(int error, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(1, error, fmt, ap);
	va_end(ap);
	exit(1);
}

int
inp_wait(int delay) {
	struct pollfd pfd[1];

	pfd[0].fd = STDIN_FILENO;
	pfd[0].events = POLLRDNORM;

	if( poll( pfd, 1, (int)(delay * 1000) ) > 0 ) {
		for(;;) {
			int ch;
			int len = read(STDIN_FILENO, &ch, 1);
			if (len == -1 && errno == EINTR)
				continue;
			if (len < 1) {
				perror("read");
				exit(1);
			}
			return ch;
		}
	}

	return 0;
}

