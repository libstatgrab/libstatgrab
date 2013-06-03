/*
 * i-scream libstatgrab
 * http://www.i-scream.org
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

#ifndef STATGRAB_TRACE_H
#define STATGRAB_TRACE_H

/* 
 * following __VA_ARGS__ helpers are taken
 * from http://stackoverflow.com/questions/5588855/standard-alternative-to-gccs-va-args-trick
 *
 * Thanks, Richard Hansen
 */

/* expands to the first argument */
#define FIRST(...) FIRST_HELPER(__VA_ARGS__, throwaway)
#define FIRST_HELPER(first, ...) first

/*
 * if there's only one argument, expands to nothing.  if there is more
 * than one argument, expands to a comma followed by everything but
 * the first argument.  only supports up to 9 arguments but can be
 * trivially expanded.
 */
#define REST(...) REST_HELPER(NUM(__VA_ARGS__), __VA_ARGS__)
#define REST_HELPER(qty, ...) REST_HELPER2(qty, __VA_ARGS__)
#define REST_HELPER2(qty, ...) REST_HELPER_##qty(__VA_ARGS__)
#define REST_HELPER_ONE(first)
#define REST_HELPER_TWOORMORE(first, ...) , __VA_ARGS__
#define NUM(...) \
    SELECT_10TH(__VA_ARGS__, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE,\
                TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, ONE, throwaway)
#define SELECT_10TH(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, ...) a10

#ifdef WITH_LIBLOG4CPLUS
#include <log4cplus/clogger.h>
#define LOGMSG(module, ll, msg) log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), ll, msg)
#define LOGMSG_FMT(module, ll, ...) log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), ll, __VA_ARGS__ )

#define PANIC_LOG(module, msg) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_FATAL_LOG_LEVEL, msg " (at %s:%d)", __FILE__, __LINE__); \
	exit(255); \
} while(0)
#define PANIC_LOG_FMT(module, ...) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_FATAL_LOG_LEVEL, FIRST(__VA_ARGS__) " (at %s:%d)" REST(__VA_ARGS__), __FILE__, __LINE__); \
	exit(255); \
} while(0)

#define ERROR_LOG(module, msg) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_ERROR_LOG_LEVEL, msg " (at %s:%d)", __FILE__, __LINE__); \
} while(0)
#define ERROR_LOG_FMT(module, ...) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_ERROR_LOG_LEVEL, FIRST(__VA_ARGS__) " (at %s:%d)" REST(__VA_ARGS__), __FILE__, __LINE__); \
} while(0)

#define WARN_LOG(module, msg) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_WARN_LOG_LEVEL, msg " (at %s:%d)", __FILE__, __LINE__); \
} while(0)
#define WARN_LOG_FMT(module, ...) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_WARN_LOG_LEVEL, FIRST(__VA_ARGS__) " (at %s:%d)" REST(__VA_ARGS__), __FILE__, __LINE__); \
} while(0)

#define INFO_LOG(module, msg) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_INFO_LOG_LEVEL, msg " (at %s:%d)", __FILE__, __LINE__); \
} while(0)
#define INFO_LOG_FMT(module, ...) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_INFO_LOG_LEVEL, FIRST(__VA_ARGS__) " (at %s:%d)" REST(__VA_ARGS__), __FILE__, __LINE__); \
} while(0)

#define DEBUG_LOG(module, msg) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_DEBUG_LOG_LEVEL, msg " (at %s:%d)", __FILE__, __LINE__); \
} while(0)
#define DEBUG_LOG_FMT(module, ...) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_DEBUG_LOG_LEVEL, FIRST(__VA_ARGS__) " (at %s:%d)" REST(__VA_ARGS__), __FILE__, __LINE__); \
} while(0)

#define TRACE_LOG(module, msg) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_TRACE_LOG_LEVEL, msg " (at %s:%d)", __FILE__, __LINE__); \
} while(0)
#define TRACE_LOG_FMT(module, ...) do { \
	log4cplus_logger_log(LOG4CPLUS_TEXT("statgrab." module), L4CP_TRACE_LOG_LEVEL, FIRST(__VA_ARGS__) " (at %s:%d)" REST(__VA_ARGS__), __FILE__, __LINE__); \
} while(0)
#else
#define LOGMSG(module, ll, msg) ((void)0)
#define LOGMSG_FMT(module, ll, ...) ((void)0)

#define PANIC_LOG(module, msg) do { fprintf( stderr, "panic condition: " msg " in %s at %s:%d", module, __FILE__, __LINE__); exit(255); } while(0)
#define PANIC_LOG_FMT(module, ...) do { fprintf( stderr, "panic condition: " FIRST(__VA_ARGS__) " in %s at %s:%d" REST(__VA_ARGS__), module, __FILE__, __LINE__); exit(255); } while(0)

#define ERROR_LOG(module, msg) ((void)0)
#define ERROR_LOG_FMT(module, ...) ((void)0)

#define WARN_LOG(module, msg) ((void)0)
#define WARN_LOG_FMT(module, ...) ((void)0)

#define INFO_LOG(module, msg) ((void)0)
#define INFO_LOG_FMT(module, ...) ((void)0)

#define DEBUG_LOG(module, msg) ((void)0)
#define DEBUG_LOG_FMT(module, ...) ((void)0)

#define TRACE_LOG(module, msg) ((void)0)
#define TRACE_LOG_FMT(module, ...) ((void)0)
#endif

#endif /* STATGRAB_TRACE_H */
