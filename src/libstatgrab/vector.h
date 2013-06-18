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

#ifndef STATGRAB_VECTOR_H
#define STATGRAB_VECTOR_H

#include <stdlib.h>

typedef void (*vector_init_function)(void *item);
typedef sg_error (*vector_copy_function)(const void *src, void *dst);
typedef sg_error (*vector_compute_diff_function)(void *dst, const void *src);
typedef int (*vector_compare_function)(const void *a, const void *b);
typedef void (*vector_destroy_function)(void *item);

typedef struct sg_vector_init_info {
	size_t item_size;
	vector_init_function init_fn;
	vector_copy_function copy_fn;
	vector_compute_diff_function compute_diff_fn;
	vector_compare_function compare_fn;
	vector_destroy_function destroy_fn;
} sg_vector_init_info;

#define VECTOR_INIT_INFO_FULL_INIT(type) \
	struct sg_vector_init_info type##_vector_init_info = { \
		sizeof(type), \
		(vector_init_function)(type##_item_init), \
		(vector_copy_function)(type##_item_copy), \
		(vector_compute_diff_function)(type##_item_compute_diff), \
		(vector_compare_function)(type##_item_compare), \
		(vector_destroy_function)(type##_item_destroy) \
	}

#define VECTOR_INIT_INFO_EMPTY_INIT(type) \
	struct sg_vector_init_info type##_vector_init_info = { sizeof(type), NULL, NULL, NULL, NULL, NULL }

#define VECTOR_INIT_INFO(type) type##_vector_init_info

#ifndef NDEBUG
typedef struct sg_vector {
	size_t start_eye;
	size_t used_count;
	size_t alloc_count;
	size_t block_shift;
	struct sg_vector_init_info info;
	size_t final_eye;
} sg_vector;
__sg_private sg_error sg_prove_vector(const sg_vector *vec);
#else
typedef struct sg_vector {
	size_t used_count;
	size_t alloc_count;
	size_t block_shift;
	struct sg_vector_init_info info;
} sg_vector;
#define sg_prove_vector(x) SG_ERROR_NONE
#endif

/* Create a vector. Returns address to vector on success or NULL on
 * out-of-memory. Allocates space for initially block_size items.
 */
__sg_private struct sg_vector *sg_vector_create(size_t block_size, size_t alloc_count, size_t initial_used, const sg_vector_init_info * const info);
/* clear a vector - but do not free the memory itself */
__sg_private void sg_vector_clear(struct sg_vector *vector);

#define VECTOR_CREATE(type, block_size) \
	sg_vector_create((size_t)(block_size), (size_t)(block_size), 0, & VECTOR_INIT_INFO(type))
#define VECTOR_CLEAR(vector) \
	sg_vector_clear(vector)

/* Resize a vector. Returns new vector on success, NULL on out-of-memory.
 * On out-of-memory, the old contents of the vector will be destroyed and
 * the old vector will be freed. */
__sg_private struct sg_vector *sg_vector_resize(struct sg_vector *vector, size_t new_count);

/* Free a previously allocated vector. */
__sg_private void sg_vector_free(struct sg_vector *vector);

#define VECTOR_CREATE_OR_RESIZE(vector, new_count, type) \
	vector ? sg_vector_resize(vector, (size_t)(new_count)) \
	       : sg_vector_create((size_t)(new_count), (size_t)(new_count), \
				  (size_t)(new_count), & VECTOR_INIT_INFO(type))

#ifdef STRUCT_SG_VECTOR_ALIGN_OK
#define VECTOR_SIZE ((size_t)(((struct sg_vector *)0) + ((size_t)1)))
#define VECTOR_DATA(vector) \
	(vector ? (void *)&((vector)[1]) : NULL)
#define VECTOR_DATA_CONST(vector) \
	(vector ? (void const *)&((vector)[1]) : NULL)

#define VECTOR_ADDR_ARITH(ptr) \
	&(((struct sg_vector *)ptr)[-1])
#define VECTOR_CONST_ADDR_ARITH(ptr) \
	&(((struct sg_vector const *)ptr)[-1])
#else
struct sg_vector_size_helper {
	struct sg_vector v;
	long long ll;
};

#define VECTOR_SIZE offsetof(struct sg_vector_size_helper,ll)

/* Return the data ptr of a vector */
#define VECTOR_DATA(vector) \
	(vector ? (void *)(((char *)vector)+VECTOR_SIZE) : NULL)
#define VECTOR_DATA_CONST(vector) \
	(vector ? (void const *)(((char *)vector)+VECTOR_SIZE) : NULL)

#define VECTOR_ADDR_ARITH(ptr) \
	(sg_vector *)(((char *)(ptr))-VECTOR_SIZE)
#define VECTOR_CONST_ADDR_ARITH(ptr) \
	(struct sg_vector const *)(((char const *)(ptr))-VECTOR_SIZE)
#endif

/* Return the vector for a data */
#define VECTOR_ADDRESS(ptr) \
	(ptr ? (SG_ERROR_NONE == sg_prove_vector(VECTOR_ADDR_ARITH(ptr)) ? VECTOR_ADDR_ARITH(ptr) : NULL ) : NULL)
#define VECTOR_ADDRESS_CONST(ptr) \
	(ptr ? (SG_ERROR_NONE == sg_prove_vector(VECTOR_CONST_ADDR_ARITH(ptr)) ? VECTOR_CONST_ADDR_ARITH(ptr) : NULL ) : NULL)

/* Return the current size of a vector. */
#define VECTOR_ITEM_COUNT(vector) \
	(vector ? (vector)->used_count : 0)

#define VECTOR_UPDATE_ERROR_CLEANUP

#define VECTOR_UPDATE(vectorptr,new_count,data,datatype) \
	do { \
		if( NULL != (*(vectorptr) = VECTOR_CREATE_OR_RESIZE(*(vectorptr), new_count, datatype)) ){ \
			assert(VECTOR_ITEM_COUNT(*(vectorptr)) == ((size_t)(new_count))); \
			data = (datatype *)(VECTOR_DATA(*(vectorptr))); \
		} \
		else if( !new_count ) {\
			data = NULL; \
		} \
		else {\
			VECTOR_UPDATE_ERROR_CLEANUP \
			return sg_get_error(); \
		} \
	} while( 0 )

__sg_private sg_vector *sg_vector_clone(const sg_vector *src);
__sg_private sg_error sg_vector_clone_into(sg_vector **dest, const sg_vector *src);

__sg_private sg_error sg_vector_compute_diff(sg_vector **dest_vector_ptr, const sg_vector *cur_vector, const sg_vector *last_vector);

#endif /* ?STATGRAB_VECTOR_H */
