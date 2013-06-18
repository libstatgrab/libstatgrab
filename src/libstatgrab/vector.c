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

#ifndef NDEBUG
#define SG_VECTOR_START_EYE MAGIC_EYE('s','g','v','s')
#define SG_VECTOR_FINAL_EYE MAGIC_EYE('f','v','g','s')
#endif

static void
sg_vector_destroy_unused(struct sg_vector *vector, size_t new_count){

	/* Destroy any now-unused items.
	 *
	 * Note that there's an assumption here that making the vector smaller
	 * will never fail; if it did, then we would have destroyed items here
	 * but not actually got rid of the vector pointing to them before the
	 * error return.) */
	if (new_count < vector->used_count && vector->info.destroy_fn != NULL) {
		char *data = VECTOR_DATA(vector);
		size_t i;

		i = vector->used_count;
		while( i > new_count ) {
			--i;
			vector->info.destroy_fn(data + i * vector->info.item_size);
		}
	}

	if (new_count < vector->used_count)
		vector->used_count = new_count;
}

static void
sg_vector_init_new(struct sg_vector *vector, size_t new_count){
	/* Initialise any new items. */
	if (new_count > vector->used_count && vector->info.init_fn != NULL) {
		char *data = VECTOR_DATA(vector);
		size_t i;

		for( i = vector->used_count; i < new_count; ++i ){
			vector->info.init_fn(data + i * vector->info.item_size);
		}
	}

	if (new_count > vector->used_count)
		vector->used_count = new_count;
}

sg_error
sg_prove_vector(const sg_vector *vec) {
	TRACE_LOG_FMT("vector", "sg_prove_vector(%p)", vec);
	assert( SG_VECTOR_START_EYE == vec->start_eye );
	assert( SG_VECTOR_FINAL_EYE == vec->final_eye );
	return SG_ERROR_NONE;
}

static sg_error
sg_prove_vector_compat(const sg_vector *one, const sg_vector *two) {
	TRACE_LOG_FMT("vector", "sg_prove_vector_compat(%p, %p)", one, two);
	assert( one );
	assert( two );
	assert( ( one->info.item_size == two->info.item_size ) &&
		( one->info.init_fn == two->info.init_fn ) &&
		( one->info.copy_fn == two->info.copy_fn ) &&
		( one->info.compute_diff_fn == two->info.compute_diff_fn ) &&
		( one->info.compare_fn == two->info.compare_fn ) &&
		( one->info.destroy_fn == two->info.destroy_fn ) );

	return SG_ERROR_NONE;
}

#define NEXT_POWER2_IN(x,bs) ( ((( (x) - 1 ) >> (bs)) + 1) << (bs) )

static struct sg_vector *
sg_vector_create_int(size_t block_shift, size_t alloc_count, size_t initial_used,
		     const sg_vector_init_info * const info) {

	size_t round_count = NEXT_POWER2_IN(MAX(alloc_count, initial_used), block_shift);
	struct sg_vector *vector = sg_realloc(NULL, VECTOR_SIZE + round_count * info->item_size);
	if(vector) {
		TRACE_LOG_FMT("vector", "vector for %lu items allocated at %p", round_count, vector);
#ifndef NDEBUG
		vector->start_eye = SG_VECTOR_START_EYE;
		vector->final_eye = SG_VECTOR_FINAL_EYE;
#endif
		vector->info = *info;
		vector->block_shift = block_shift;
		vector->alloc_count = 1LU << block_shift;
		vector->used_count = 0;

		sg_vector_init_new(vector, initial_used);
	}

	return vector;
}

struct sg_vector *
sg_vector_create(size_t block_size, size_t alloc_count, size_t initial_used,
		 const sg_vector_init_info * const info) {

	size_t block_shift = 0;

	TRACE_LOG_FMT("vector", "allocating vector fitting for %lu items and use %lu of them", alloc_count, initial_used);

	while( ((unsigned)1 << block_shift) < block_size )
		++block_shift;

	return sg_vector_create_int( block_shift, alloc_count, initial_used, info );
}

void
sg_vector_free(struct sg_vector *vector) {

	if( !vector )
		return;

	if(SG_ERROR_NONE == sg_prove_vector(vector)) {
		TRACE_LOG_FMT("vector", "freeing vector %p containing %lu items", vector, vector->used_count);

		sg_vector_destroy_unused(vector, 0);
		free( vector );
	}
}

void
sg_vector_clear(struct sg_vector *vector) {
	if( !vector )
		return;

	if(SG_ERROR_NONE == sg_prove_vector(vector)) {
		TRACE_LOG_FMT("vector", "clearing vector %p containing %lu items", vector, vector->used_count);

		sg_vector_destroy_unused(vector, 0);
	}
}

struct sg_vector *
sg_vector_resize(struct sg_vector *vector, size_t new_count) {

	size_t round_count;

	assert(vector);
	if( !vector ) {
		SET_ERROR("vector", SG_ERROR_INVALID_ARGUMENT, "sg_vector_resize: invalid argument value for vector");
		return NULL;
	}

	if(SG_ERROR_NONE != sg_prove_vector(vector)) {
		SET_ERROR("vector", SG_ERROR_INVALID_ARGUMENT, "sg_vector_resize: invalid vector");
		return NULL;
	}

	TRACE_LOG_FMT("vector", "resizing vector(%p) using %lu items to fit %lu items", vector, vector->used_count, new_count);

	if( 0U == new_count ){
		sg_vector_free(vector);
		sg_clear_error();
		return NULL;
	}

	sg_vector_destroy_unused(vector, new_count);

	/* Round up the desired size to the next multiple of the block size. */
	round_count = NEXT_POWER2_IN(new_count, vector->block_shift);

	/* Resize the vector if necessary. */
	if (round_count != vector->alloc_count) {
		sg_vector *new_vector;

		new_vector = sg_realloc(vector, VECTOR_SIZE + round_count * vector->info.item_size);
		if (new_vector == NULL && round_count != 0U) {
			/* Out of memory -- free the contents of the vector. */
			sg_vector_free(vector);
			return NULL;
		}

		vector = new_vector;
		vector->alloc_count = round_count;
	}

	sg_vector_init_new(vector, new_count);

	TRACE_LOG_FMT("vector", "resized vector(%p) to fit %lu items", vector, round_count);

	return vector;
}

size_t
sg_get_nelements(const void *data){
    const sg_vector *vector = VECTOR_ADDRESS(data);
    return vector ? vector->used_count : 0;
}

static sg_error
sg_vector_clone_into_int(sg_vector **dest, const sg_vector *src){

	/* we can assume that everything is correct from caller perspective */
	size_t i;
	sg_vector *tmp   = ((*dest)->used_count == src->used_count)
			 ? *dest
			 : sg_vector_resize(*dest, src->used_count);
	char *src_data   = VECTOR_DATA(src), *dest_data = VECTOR_DATA(tmp);
	size_t item_size = src->info.item_size;

	assert(src->info.copy_fn);

	if( !tmp ) {
		RETURN_FROM_PREVIOUS_ERROR( "vector", sg_get_error() );
	}

	for( i = 0; i < src->used_count; ++i ) {
		sg_error rc = src->info.copy_fn( dest_data + i * item_size, src_data + i * item_size );
		if( SG_ERROR_NONE != rc ) {
			sg_vector_free( tmp );
			*dest = NULL;
			return rc;
		}
	}

	*dest = tmp;

	return SG_ERROR_NONE;
}

sg_error
sg_vector_clone_into(sg_vector **dest, const sg_vector *src){

	if( !dest ) {
		RETURN_WITH_SET_ERROR( "vector", SG_ERROR_INVALID_ARGUMENT, "dest" );
	}

	if( !src ) {
		if(*dest) {
			sg_vector_free(*dest);
			*dest = NULL;
		}

		return SG_ERROR_NONE;
	}

	if(SG_ERROR_NONE != sg_prove_vector(src)) {
		RETURN_WITH_SET_ERROR( "vector", SG_ERROR_INVALID_ARGUMENT, "src" );
	}

	if( !*dest ) {
		*dest = sg_vector_clone( src );
		if( *dest )
			return SG_ERROR_NONE;
	}
	else if( ( SG_ERROR_NONE == sg_prove_vector(*dest) ) &&
		 ( SG_ERROR_NONE == sg_prove_vector_compat(*dest, src) ) &&
		 ( SG_ERROR_NONE == sg_vector_clone_into_int( dest, src ) ) ) {

		return SG_ERROR_NONE;
	}

	sg_vector_free(*dest);
	*dest = NULL;
	return sg_get_error();
}

sg_vector *
sg_vector_clone(const sg_vector *src){

	sg_vector *dest = NULL;

	if( !src )
		return NULL;

	if( ( SG_ERROR_NONE == sg_prove_vector(src) ) &&
	    ( ( dest = sg_vector_create_int( src->block_shift, src->alloc_count, src->used_count, &src->info ) ) != NULL ) &&
	    ( SG_ERROR_NONE == sg_vector_clone_into_int( &dest, src ) ) ) {

		TRACE_LOG_FMT("vector", "cloned vector containing %lu items", src->used_count);

		return dest;
	}

	sg_vector_free(dest);
	return NULL;
}

sg_error
sg_free_stats_buf(void *data){

	if(NULL != data) {
		sg_vector *data_vector = VECTOR_ADDRESS(data);
		sg_vector_free(data_vector);
		return SG_ERROR_NONE;
	}

	return SG_ERROR_NONE;
}

sg_error
sg_vector_compute_diff(sg_vector **dest_vector_ptr, const sg_vector *cur_vector, const sg_vector *last_vector) {

	if( NULL == dest_vector_ptr ) {
		RETURN_WITH_SET_ERROR("vector", SG_ERROR_INVALID_ARGUMENT, "sg_vector_compute_diff(dest_vector_ptr)");
	}

	if( cur_vector ) {
		sg_error rc = sg_vector_clone_into( dest_vector_ptr, cur_vector ); /* proves *dest, cur */
		if( SG_ERROR_NONE != rc ) {
			RETURN_FROM_PREVIOUS_ERROR( "vector", rc );
		}

		if( NULL == *dest_vector_ptr )
			return SG_ERROR_NONE;
		assert( cur_vector->info.compute_diff_fn );
		assert( cur_vector->info.compare_fn );

		TRACE_LOG_FMT("vector",
			      "computing vector diff between vector containing %lu items (cur) and vector containing %lu items (last)",
			      cur_vector->used_count, last_vector ? last_vector->used_count : 0 );

		if( last_vector &&
		    ( SG_ERROR_NONE == sg_prove_vector(last_vector) ) &&
		    ( SG_ERROR_NONE == sg_prove_vector_compat(cur_vector, last_vector ) ) ) {
			size_t i, item_size = last_vector->info.item_size;
			unsigned matched[(cur_vector->used_count / (8 * sizeof(unsigned))) + 1];

			char *diff = VECTOR_DATA(*dest_vector_ptr),
			     *last = VECTOR_DATA(last_vector);

			memset( matched, 0, sizeof(matched) );

			for( i = 0; i < (*dest_vector_ptr)->used_count; ++i ) {
				size_t j;
				for( j = 0; j < last_vector->used_count; ++j ) {
					if( BIT_ISSET(matched, j) ) /* already matched? */
						continue;

					if (last_vector->info.compare_fn(diff + i * item_size, last + j * item_size) == 0) {
						BIT_SET(matched, j);
						last_vector->info.compute_diff_fn(last + j * item_size, diff + i * item_size);
					}
				}

				if( j == last_vector->used_count ) {
					/* We have lost an item since last time - skip */
				}
			}

			/* XXX append the items lost since last run? */
		}
	}
	else {
		sg_vector_free(*dest_vector_ptr);
		*dest_vector_ptr = NULL;

		RETURN_WITH_SET_ERROR("vector", SG_ERROR_INVALID_ARGUMENT, "sg_vector_compute_diff(cur_vector)");
	}

	return SG_ERROR_NONE;
}
