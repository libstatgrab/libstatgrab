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

int statgrab_vector_resize(void **vector, vector_header *h, int count);

/* Declare a vector. Specify the init/destroy functions as NULL if you don't
 * need them. The block size is how many items to allocate at once. */
#define VECTOR_DECLARE(name, item_type, block_size, init_fn, destroy_fn) \
	item_type * name = NULL; \
	vector_header name##_header = { \
		sizeof(item_type), \
		0, \
		0, \
		block_size, \
		(vector_init_function) init_fn, \
		(vector_destroy_function) destroy_fn \
	}

/* Return the current size of a vector. */
#define VECTOR_SIZE(name) \
	name##_header.used_count

/* Resize a vector. Returns 0 on success, -1 on out-of-memory. On
 * out-of-memory, the old contents of the vector will be destroyed and the old
 * vector will be freed. */
#define VECTOR_RESIZE(name, num_items) \
	statgrab_vector_resize((void **) &name, &name##_header, num_items)

/* Free a vector, destroying its contents. */
#define VECTOR_FREE(name) \
	VECTOR_RESIZE(name, 0)


