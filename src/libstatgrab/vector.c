/*
 * i-scream central monitoring system
 * http://www.i-scream.org
 * Copyright (C) 2000-2004 i-scream
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "vector.h"

void *statgrab_vector_resize(void *vector, vector_header *h, int count) {
	int new_count, i;

	/* Destroy any now-unused items. */
	if (count < h->used_count && h->destroy_fn != NULL) {
		for (i = count; i < h->used_count; i++) {
			h->destroy_fn((void *) (vector + i * h->item_size));
		}
	}

	/* Round up the desired size to the next multiple of the block size. */
	new_count =  ((count - 1 + h->block_size) / h->block_size)
	             * h->block_size;

	/* Resize the vector if necessary. */
	if (new_count != h->alloc_count) {
		char *new_vector;

		new_vector = realloc(vector, new_count * h->item_size);
		if (new_vector == NULL && new_count != 0) {
			/* Out of memory -- free the contents of the vector. */
			statgrab_vector_resize(vector, h, 0);
			/* And return the sentinel value to indicate failure. */
			return statgrab_vector_sentinel;
		}

		vector = new_vector;
		h->alloc_count = new_count;
	}

	/* Initialise any new items. */
	if (count > h->used_count && h->init_fn != NULL) {
		for (i = h->used_count; i < count; i++) {
			h->init_fn((void *) (vector + i * h->item_size));
		}
	}

	h->used_count = count;

	return vector;
}

