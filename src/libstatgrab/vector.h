#include <stdlib.h>

typedef void (*vector_init_function)(void *item);
typedef void (*vector_destroy_function)(void *item);

typedef struct {
	size_t item_size;
	int used_count;
	int alloc_count;
	int block_size;
	vector_init_function init_fn;
	vector_destroy_function destroy_fn;
} vector_header;

#define VECTOR_HEADER(name, item_type, block_size, init_fn, destroy_fn) \
	vector_header name##_header = { \
		sizeof(item_type), \
		0, \
		0, \
		block_size, \
		(vector_init_function) init_fn, \
		(vector_destroy_function) destroy_fn \
	}

/* A pointer value that won't be returned by malloc, and isn't NULL, so can be
 * used as a sentinel by allocation routines that need to return NULL
 * sometimes. */
extern char statgrab_vector_sentinel_value;
static void *statgrab_vector_sentinel = &statgrab_vector_sentinel_value;

/* Internal function to resize the vector. Ideally it would take void ** as the
 * first parameter, but ANSI strict-aliasing rules would then prevent it from
 * doing anything with it, so we return a sentinel value as above instead if
 * allocation fails. */
void *statgrab_vector_resize(void *vector, vector_header *h, int count);

/* Declare a vector. Specify the init/destroy functions as NULL if you don't
 * need them. The block size is how many items to allocate at once. */
#define VECTOR_DECLARE(name, item_type, block_size, init_fn, destroy_fn) \
	item_type *name = NULL; \
	VECTOR_HEADER(name, item_type, block_size, init_fn, destroy_fn)

/* As VECTOR_DECLARE, but for a static vector. */
#define VECTOR_DECLARE_STATIC(name, item_type, block_size, init_fn, destroy_fn) \
	static item_type *name = NULL; \
	static VECTOR_HEADER(name, item_type, block_size, init_fn, destroy_fn)

/* Return the current size of a vector. */
#define VECTOR_SIZE(name) \
	name##_header.used_count

/* Resize a vector. Returns 0 on success, -1 on out-of-memory. On
 * out-of-memory, the old contents of the vector will be destroyed and the old
 * vector will be freed.
 *
 * This is ugly because it needs to check for the sentinel value.
 */
#define VECTOR_RESIZE(name, num_items) \
	(((name = statgrab_vector_resize((char *) name, &name##_header, num_items)) \
	  == statgrab_vector_sentinel) ? -1 : 0)

/* Free a vector, destroying its contents. */
#define VECTOR_FREE(name) \
	VECTOR_RESIZE(name, 0)

