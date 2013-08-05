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

#ifndef __SG_EXAMPLES_HELPERS_H__
#define __SG_EXAMPLES_HELPERS_H__

#include <signal.h>
#include <errno.h>

int register_sig_flagger(int signo, int *flag_ptr);
int sg_warn(const char *prefix);
void sg_die(const char *prefix, int exit_code);
void die(int error, const char *fmt, ...);
int inp_wait(int delay);

#endif /* __SG_EXAMPLES_HELPERS_H__ */
