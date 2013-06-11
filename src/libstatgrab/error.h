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
#ifndef STATGRAB_ERROR_H
#define STATGRAB_ERROR_H

__sg_private void sg_clear_error(void);
__sg_private sg_error sg_set_error_fmt(sg_error code, const char *arg, ...);
__sg_private sg_error sg_set_error_with_errno_fmt(sg_error code, const char *arg, ...);
__sg_private sg_error sg_set_error_with_errno_code_fmt(sg_error code, int errno_value, const char *arg, ...);

#define SET_ERROR(comp, code, ...) \
do { \
	char *buf = NULL; \
	sg_set_error_fmt(code, __VA_ARGS__); \
	ERROR_LOG_FMT(comp, "%s", sg_strperror(&buf, NULL)); \
	free(buf); \
} while(0)

#define RETURN_WITH_SET_ERROR(comp, code, ...) SET_ERROR(comp, code, __VA_ARGS__); return code

#define SET_ERROR_WITH_ERRNO(comp, code, ...) \
do { \
	char *buf = NULL; \
	sg_set_error_with_errno_fmt(code, __VA_ARGS__); \
	ERROR_LOG_FMT(comp, "%s", sg_strperror(&buf, NULL)); \
	free(buf); \
} while(0)

#define RETURN_WITH_SET_ERROR_WITH_ERRNO(comp, code, ...) SET_ERROR_WITH_ERRNO(comp, code, __VA_ARGS__); return code

#define SET_ERROR_WITH_ERRNO_CODE(comp, code, errno_value, ...) \
do { \
	char *buf = NULL; \
	sg_set_error_with_errno_code_fmt(code, errno_value, __VA_ARGS__); \
	ERROR_LOG_FMT(comp, "%s", sg_strperror(&buf, NULL)); \
	free(buf); \
} while(0)

#define RETURN_WITH_SET_ERROR_WITH_ERRNO_CODE(comp, code, errno_value, ...) SET_ERROR_WITH_ERRNO_CODE(comp, code, errno_value, __VA_ARGS__); return code

#define RETURN_FROM_PREVIOUS_ERROR(comp, code) \
do { \
	char *buf = NULL; \
	ERROR_LOG_FMT(comp, "%s", sg_strperror(&buf, NULL)); \
	free(buf); \
} while(0); return code

#endif /* STATGRAB_ERROR_H */
