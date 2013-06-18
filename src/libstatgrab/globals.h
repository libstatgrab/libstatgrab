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
#ifndef STATGRAB_GLOBALS_H
#define STATGRAB_GLOBALS_H

typedef sg_error (*comp_global_init_function)(unsigned id);
typedef void (*comp_global_destroy_function)(void);
typedef void (*comp_global_cleanup_function)(void *);

struct sg_comp_status {
	sg_error init_error;
};

#if defined(ENABLE_THREADS) && defined(HAVE_PTHREAD)
#define DECLARE_REQUIRED_COMP_LOCKS(comp,...) static const char * sg_##comp##_lock_names[] = { __VA_ARGS__ };
#define REQUIRED_COMP_LOCKS(comp) sg_##comp##_lock_names,
#else
#define DECLARE_REQUIRED_COMP_LOCKS(comp,...)
#define REQUIRED_COMP_LOCKS(comp)
#endif

struct sg_comp_init {
	comp_global_init_function init_fn;
	comp_global_destroy_function destroy_fn;
	comp_global_cleanup_function cleanup_fn;
	size_t static_buf_size;
#if defined(ENABLE_THREADS) && defined(HAVE_PTHREAD)
	const char **required_locks;
#endif
	struct sg_comp_status *status;
};

void *sg_comp_get_tls(unsigned id);

#define DEFAULT_INIT_COMP(comp,...)			\
	static sg_error sg_##comp##_init_comp(unsigned);\
	static void sg_##comp##_destroy_comp(void);	\
	static void sg_##comp##_cleanup_comp(void *);	\
							\
	static struct sg_comp_status sg_##comp##_status;\
	DECLARE_REQUIRED_COMP_LOCKS(comp, __VA_ARGS__)	\
							\
	__sg_private				    \
	const struct sg_comp_init sg_##comp##_init = {	\
		sg_##comp##_init_comp,			\
		sg_##comp##_destroy_comp,		\
		sg_##comp##_cleanup_comp,		\
		sizeof(struct sg_##comp##_glob),	\
		REQUIRED_COMP_LOCKS(comp)		\
		&sg_##comp##_status			\
	};						\
	static unsigned int sg_##comp##_glob_id

#define GLOBAL_GET_TLS(comp) (struct sg_##comp##_glob *)(sg_comp_get_tls(sg_##comp##_glob_id))
#define GLOBAL_SET_ID(comp,id) sg_##comp##_glob_id = (id)

#define EASY_COMP_INIT_FN(comp)				\
	static sg_error sg_##comp##_init_comp(unsigned id){ \
		GLOBAL_SET_ID(comp,id);			\
		return SG_ERROR_NONE;			\
	}

#define EASY_COMP_DESTROY_FN(comp)			\
	static void sg_##comp##_destroy_comp(void){}

#define EASY_COMP_CLEANUP_FN(comp,nvect)		\
	static void sg_##comp##_cleanup_comp(void *p){	\
		struct sg_##comp##_glob *comp##_glob = p; \
		unsigned i;				\
		TRACE_LOG_FMT(#comp, "sg_" #comp "_cleanup_comp(%p)", p); \
		assert(comp##_glob);			\
		for( i = 0; i < nvect; ++i )		\
			sg_vector_free(comp##_glob->comp##_vectors[i]); \
		memset(comp##_glob->comp##_vectors, 0, sizeof(comp##_glob->comp##_vectors) ); \
	}

#define EASY_COMP_SETUP(comp,nvect,...)			\
	struct sg_##comp##_glob {			\
		sg_vector *comp##_vectors[nvect];	\
	};						\
							\
	static unsigned int sg_##comp##_glob_id;	\
							\
	EASY_COMP_INIT_FN(comp)				\
	EASY_COMP_DESTROY_FN(comp)			\
	EASY_COMP_CLEANUP_FN(comp,nvect)		\
							\
	static struct sg_comp_status sg_##comp##_status;\
	DECLARE_REQUIRED_COMP_LOCKS(comp, __VA_ARGS__)	\
							\
	__sg_private				    \
	const struct sg_comp_init sg_##comp##_init = {	\
		sg_##comp##_init_comp,			\
		sg_##comp##_destroy_comp,		\
		sg_##comp##_cleanup_comp,		\
		sizeof(struct sg_##comp##_glob),	\
		REQUIRED_COMP_LOCKS(comp)		\
		&sg_##comp##_status			\
	}

#define EXTENDED_COMP_SETUP(comp,nvect,...)		\
	static sg_error sg_##comp##_init_comp(unsigned);\
	static void sg_##comp##_destroy_comp(void);	\
	static void sg_##comp##_cleanup_comp(void *);	\
							\
	struct sg_##comp##_glob {			\
		sg_vector *comp##_vectors[nvect];	\
	};						\
							\
	static unsigned int sg_##comp##_glob_id;	\
	DECLARE_REQUIRED_COMP_LOCKS(comp, __VA_ARGS__)	\
							\
	static struct sg_comp_status sg_##comp##_status;\
	__sg_private				    \
	const struct sg_comp_init sg_##comp##_init = {	\
		sg_##comp##_init_comp,			\
		sg_##comp##_destroy_comp,		\
		sg_##comp##_cleanup_comp,		\
		sizeof(struct sg_##comp##_glob),	\
		REQUIRED_COMP_LOCKS(comp)		\
		&sg_##comp##_status			\
	}

#define EASY_COMP_ACCESS(fn,comp,name,idx)		\
	sg_##name##_stats *fn(size_t *entries) {	\
							\
		struct sg_##comp##_glob *comp##_glob = GLOBAL_GET_TLS(comp); \
		sg_error rc;				\
		TRACE_LOG(#comp, "entering " #fn); \
		if( !comp##_glob ) {			\
			/* assuming error comp can't neither */ \
			ERROR_LOG(#comp, #fn " failed - cannot get glob"); \
			if(entries)			\
				*entries = 0;		\
			return NULL;			\
		}					\
							\
		if(!comp##_glob->comp##_vectors[idx])	\
			comp##_glob->comp##_vectors[idx] = sg_vector_create(1, 1, 1, & VECTOR_INIT_INFO(sg_##name##_stats)); \
							\
		if(comp##_glob->comp##_vectors[idx]){	\
			sg_##name##_stats *name##_stats = (sg_##name##_stats *)VECTOR_DATA(comp##_glob->comp##_vectors[idx]); \
			TRACE_LOG_FMT(#comp, "calling " #fn "_int(%p)", name##_stats); \
			rc = fn##_int(name##_stats);	\
			if(SG_ERROR_NONE == rc){	\
				TRACE_LOG(#comp, #fn " succeded"); \
				sg_clear_error();	\
				if(entries)		\
					*entries = VECTOR_ITEM_COUNT(comp##_glob->comp##_vectors[idx]); \
				return name##_stats;	\
			}				\
		}					\
		else {					\
			rc = sg_get_error();		\
		}					\
							\
		WARN_LOG_FMT(#comp, #fn " failed with %s", sg_str_error(rc)); \
							\
		if(entries)				\
			*entries = 0;			\
							\
		return NULL;				\
	}						\
							\
	sg_##name##_stats *fn##_r(size_t *entries) {	\
							\
		sg_vector *name##_stats_vector = sg_vector_create(1, 1, 1, & VECTOR_INIT_INFO(sg_##name##_stats)); \
		sg_error rc;				\
		TRACE_LOG(#comp, "entering " #fn "_r"); \
		if(name##_stats_vector) {		\
			sg_##name##_stats *name##_stats = (sg_##name##_stats *)VECTOR_DATA(name##_stats_vector); \
			TRACE_LOG_FMT(#comp, "calling " #fn "_int(%p)", name##_stats); \
			rc = fn##_int(name##_stats);	\
			if(SG_ERROR_NONE == rc){ 	\
				TRACE_LOG(#comp, #fn "_r succeded"); \
				sg_clear_error();	\
				if(entries)		\
					*entries = VECTOR_ITEM_COUNT(name##_stats_vector); \
				return name##_stats;	\
			}				\
			sg_vector_free(name##_stats_vector); \
		}					\
		else {					\
			rc = sg_get_error();		\
		}					\
							\
		WARN_LOG_FMT(#comp, #fn "_r failed with %s", sg_str_error(rc)); \
							\
		if(entries)				\
			*entries = 0;			\
							\
		return NULL;				\
	}

#define MULTI_COMP_ACCESS(fn,comp,name,idx)		\
	sg_##name##_stats *fn(size_t *entries) {	\
							\
		struct sg_##comp##_glob *comp##_glob = GLOBAL_GET_TLS(comp); \
		sg_error rc;				\
		TRACE_LOG(#comp, "entering " #fn); \
		if( !comp##_glob ) {			\
			/* assuming error comp can't neither */ \
			ERROR_LOG(#comp, #fn " failed - cannot get glob"); \
			if(entries)			\
				*entries = 0;		\
			return NULL;			\
		}					\
							\
		if(!comp##_glob->comp##_vectors[idx])	\
			comp##_glob->comp##_vectors[idx] = VECTOR_CREATE(sg_##name##_stats, 16); \
		else					\
			VECTOR_CLEAR(comp##_glob->comp##_vectors[idx]); \
							\
		if(comp##_glob->comp##_vectors[idx]){	\
			TRACE_LOG_FMT(#comp, "calling " #fn "_int(%p)", &comp##_glob->comp##_vectors[idx]); \
			rc = fn##_int(&comp##_glob->comp##_vectors[idx]); \
			if(SG_ERROR_NONE == rc) {	\
				sg_##name##_stats *name##_stats = (sg_##name##_stats *)VECTOR_DATA(comp##_glob->comp##_vectors[idx]); \
				TRACE_LOG(#comp, #fn " succeded"); \
				sg_clear_error();	\
				if(entries)		\
					*entries = VECTOR_ITEM_COUNT(comp##_glob->comp##_vectors[idx]); \
				return name##_stats;	\
			}				\
		}					\
		else {					\
			rc = sg_get_error();		\
		}					\
							\
		WARN_LOG_FMT(#comp, #fn " failed with %s", sg_str_error(rc)); \
							\
		if(entries)				\
			*entries = 0;			\
							\
		return NULL;				\
	}						\
							\
	sg_##name##_stats *fn##_r(size_t *entries) {	\
		sg_vector *name##_vector = VECTOR_CREATE(sg_##name##_stats, 16); \
		sg_error rc;				\
		TRACE_LOG(#comp, "entering " #fn); \
		if(name##_vector) {			\
			TRACE_LOG_FMT(#comp, "calling " #fn "_int(%p)", &name##_vector); \
			rc = fn##_int(&name##_vector);	\
			if( SG_ERROR_NONE == rc ) {	\
				sg_##name##_stats *name##_stats = (sg_##name##_stats *)VECTOR_DATA(name##_vector); \
				TRACE_LOG(#comp, #fn " succeded"); \
				sg_clear_error();	\
				if(entries)		\
					*entries = VECTOR_ITEM_COUNT(name##_vector); \
				return name##_stats;	\
			}				\
			sg_vector_free(name##_vector);	\
		}					\
		else {					\
			rc = sg_get_error();		\
		}					\
							\
		WARN_LOG_FMT(#comp, #fn "_r failed with %s", sg_str_error(rc)); \
							\
		if(entries)				\
			*entries = 0;			\
							\
		return NULL;				\
	}

#define EASY_COMP_DIFF(fn,getfn,comp,name,diffidx,nowidx) \
	sg_##name##_stats *fn(size_t *entries) {	\
							\
		struct sg_##comp##_glob *comp##_glob = GLOBAL_GET_TLS(comp); \
		TRACE_LOG(#comp, "entering " #fn);	\
		if( !comp##_glob ) {			\
			/* assuming error comp can't neither */ \
			ERROR_LOG(#comp, #fn " failed - cannot get glob"); \
			if(entries)			\
				*entries = 0;		\
			return NULL;			\
		}					\
							\
		if(!comp##_glob->comp##_vectors[nowidx]) { \
			TRACE_LOG_FMT(#comp, #fn " has nothing to compare with - calling %s", #getfn); \
			return getfn(entries);		\
		}					\
							\
		if(!comp##_glob->comp##_vectors[diffidx]) \
			comp##_glob->comp##_vectors[diffidx] = VECTOR_CREATE(sg_##name##_stats, 1); \
							\
		if(comp##_glob->comp##_vectors[diffidx]) { \
			sg_##name##_stats name##_last = *((sg_##name##_stats *)VECTOR_DATA(comp##_glob->comp##_vectors[nowidx])); \
			sg_##name##_stats *name##_diff = (sg_##name##_stats *)VECTOR_DATA(comp##_glob->comp##_vectors[diffidx]); \
			sg_##name##_stats *name##_now = getfn(NULL); \
							\
			TRACE_LOG_FMT(#comp, "calling " #fn "_diff(%p, %p, %p)", name##_diff, name##_now, &name##_last); \
			if( (NULL != name##_now) && ( SG_ERROR_NONE == fn##_int(name##_diff, name##_now, &name##_last) ) ) { \
				TRACE_LOG(#comp, #fn " succeded"); \
				sg_clear_error();	\
				if(entries)		\
					*entries = VECTOR_ITEM_COUNT(comp##_glob->comp##_vectors[diffidx]); \
				return name##_diff;	\
			}				\
		}					\
							\
		WARN_LOG_FMT(#comp, #fn " failed with %s", sg_str_error(sg_get_error())); \
							\
		if(entries)				\
			*entries = 0;			\
							\
		return NULL;				\
	}						\
							\
	sg_##name##_stats *fn##_between(const sg_##name##_stats *name##_now, const sg_##name##_stats *name##_last, size_t *entries) { \
							\
		sg_vector *name##_diff_vector;		\
							\
		TRACE_LOG(#comp, "entering " #fn "_between"); \
		name##_diff_vector = VECTOR_CREATE(sg_##name##_stats, 1); \
		if(name##_diff_vector){			\
			sg_error rc;			\
			sg_##name##_stats *name##_diff = (sg_##name##_stats *)VECTOR_DATA(name##_diff_vector); \
			TRACE_LOG_FMT(#comp, "calling " #fn "_diff(%p, %p, %p)", name##_diff, name##_now, &name##_last); \
			rc = fn##_int(name##_diff, name##_now, name##_last); \
			if( SG_ERROR_NONE == rc ) {	\
				TRACE_LOG(#comp, #fn "_between succeded"); \
				sg_clear_error();	\
				if(entries)		\
					*entries = VECTOR_ITEM_COUNT(name##_diff_vector); \
				return name##_diff;	\
			}				\
			sg_vector_free(name##_diff_vector); \
		}					\
							\
		WARN_LOG_FMT(#comp, #fn "_between failed with %s", sg_str_error(sg_get_error())); \
							\
		if(entries)				\
			*entries = 0;			\
							\
		return NULL;				\
	}

#define MULTI_COMP_DIFF(fn,getfn,comp,name,diffidx,nowidx) \
	sg_##name##_stats *fn(size_t *entries){		\
							\
		struct sg_##comp##_glob *comp##_glob = GLOBAL_GET_TLS(comp); \
		TRACE_LOG(#comp, "entering " #fn); \
		if( !comp##_glob ) {			\
			/* assuming error comp can't neither */ \
			ERROR_LOG(#comp, #fn " failed - cannot get glob"); \
			if(entries)			\
				*entries = 0;		\
			return NULL;			\
		}					\
							\
		if(!comp##_glob->comp##_vectors[nowidx]){ \
			TRACE_LOG_FMT(#comp, #fn " has nothing to compare with - calling %s", #getfn); \
			return getfn(entries);		\
		}					\
							\
		if(!comp##_glob->comp##_vectors[diffidx]) \
			comp##_glob->comp##_vectors[diffidx] = VECTOR_CREATE(sg_##name##_stats, comp##_glob->comp##_vectors[nowidx]->used_count); \
							\
		if( comp##_glob->comp##_vectors[diffidx] ) { \
			sg_error rc;			\
			sg_vector *last = sg_vector_clone(comp##_glob->comp##_vectors[nowidx]); \
							\
			if(!last)			\
				goto err_out;		\
							\
			TRACE_LOG(#comp, "calling " #getfn "(NULL)"); \
			getfn(NULL);			\
							\
			rc = sg_vector_compute_diff(&comp##_glob->comp##_vectors[diffidx], comp##_glob->comp##_vectors[nowidx], last); \
			sg_vector_free( last );		\
							\
			if( SG_ERROR_NONE == rc ) {	\
				TRACE_LOG(#comp, #fn " succeded"); \
				sg_clear_error();	\
				if(entries)		\
					*entries = VECTOR_ITEM_COUNT(comp##_glob->comp##_vectors[diffidx]); \
				return VECTOR_DATA(comp##_glob->comp##_vectors[diffidx]); \
			}				\
		}					\
							\
	err_out:					\
		if(entries)				\
			*entries = 0;			\
							\
		WARN_LOG_FMT(#comp, #fn " failed with %s", sg_str_error(sg_get_error())); \
							\
		return NULL;				\
	}						\
							\
	sg_##name##_stats *				\
	fn##_between(const sg_##name##_stats *cur,	\
		     const sg_##name##_stats *last,	\
		     size_t *entries) {			\
							\
		sg_vector *name##_diff_vector;		\
		sg_error rc;				\
							\
		TRACE_LOG(#comp, "entering " #fn "_between"); \
		name##_diff_vector = VECTOR_CREATE(sg_##name##_stats, 1); \
		if(name##_diff_vector){			\
			rc = sg_vector_compute_diff(&name##_diff_vector, VECTOR_ADDRESS_CONST(cur), VECTOR_ADDRESS_CONST(last)); \
							\
			if( SG_ERROR_NONE == rc ) {	\
				TRACE_LOG(#comp, #fn "_between succeded"); \
				sg_clear_error();	\
				if(entries)		\
					*entries = VECTOR_ITEM_COUNT(name##_diff_vector); \
				return VECTOR_DATA(name##_diff_vector); \
			}				\
							\
			sg_vector_free(name##_diff_vector); \
		}					\
		else {					\
			rc = sg_get_error();		\
		}					\
							\
		WARN_LOG_FMT(#comp, #fn "_between failed with %s", sg_str_error(rc)); \
							\
		if(entries)				\
			*entries = 0;			\
							\
		return NULL;				\
	}

__sg_private sg_error sg_comp_init(int ignore_init_errors);
__sg_private sg_error sg_comp_destroy(void);
__sg_private sg_error sg_global_lock(void);
__sg_private sg_error sg_global_unlock(void);

#endif /* STATGRAB_GLOBALS_H */
