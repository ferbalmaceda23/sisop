#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

#include "malloc.h"
#include "printfmt.h"
#include "heap.h"

extern struct block *block_free_list;

extern int amount_of_mallocs;
extern int amount_of_frees;
extern int requested_memory;

void *
malloc(size_t requested_size)
{
	if (requested_size == 0) {
		return NULL;
	}
	size_t size = requested_size < MIN_REQUEST ? MIN_REQUEST : requested_size;
	if ((size + sizeof(struct block) + sizeof(struct region)) > LARGE_BLOCK) {
		errno = ENOMEM;
		return NULL;
	}

	struct region *next;

	// aligns to multiple of 4 bytes
	size = ALIGN4(size);

	next = find_free_region(size);
	if (next == NULL) {
		next = grow_heap(size);
	}
	if (next == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	// updates statistics
	amount_of_mallocs++;
	requested_memory += size;

	// split free regions
	if (next->size > (size + sizeof(struct region))) {
		split_region(next, size);
	}
	next->free = false;

	return REGION2PTR(next);
}

void
free(void *ptr)
{
	if (!is_in_heap(ptr))
		return;

	// updates statistics
	amount_of_frees++;

	struct region *curr = PTR2REGION(ptr);
	assert(curr->free == 0);

	curr->free = true;

	coalesce_free_regions();
	free_unused_blocks();
}

void *
calloc(size_t nmemb, size_t size)
{
	if (nmemb == 0 || size == 0)
		return NULL;
	void *allocation = malloc(nmemb * size);
	if (allocation == NULL)
		return NULL;
	memset(allocation, 0, nmemb * size);
	return allocation;
}

void *
realloc(void *ptr, size_t size)
{
	if (!ptr)
		return malloc(size);

	if (!is_in_heap(ptr))
		return NULL;

	if (size == 0) {
		free(ptr);
		return NULL;
	}

	struct region *curr = PTR2REGION(ptr);
	assert(curr->free == 0);

	void *res;
	size = ALIGN4(size < MIN_REQUEST ? MIN_REQUEST : size);

	if (curr->next != NULL && curr->next->free &&
	    (curr->next->size + curr->size) >= size) {
		curr->size += curr->next->size + sizeof(struct region);
		curr->next = curr->next->next;
	}
	if (curr->size >= size + sizeof(struct region)) {
		split_region(curr, size);
		res = ptr;
	} else {
		res = malloc(size);
		if (res == NULL)
			return NULL;

		memcpy(res, ptr, curr->size);
		free(ptr);
	}

	return res;
}
