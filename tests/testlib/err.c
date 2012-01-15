/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2010,2011 i-scream
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
#include <tools.h>
#include "testlib.h"
#include <stdarg.h>

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
