
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

#include "heap.h"
#include "printfmt.h"

void
free_unused_blocks();

bool print_statistics_set = false;

int amount_of_mallocs = 0;
int amount_of_frees = 0;
int requested_memory = 0;

struct block *block_free_list = NULL;

struct region *
first_fit(size_t size)
{
	struct block *next_block = block_free_list;
	while (next_block != NULL) {
		struct region *next_region = next_block->region;
		while (next_region != NULL) {
			if (next_region->free && next_region->size >= size) {
				return next_region;
			}
			next_region = next_region->next;
		}
		next_block = next_block->next;
	}
	return NULL;
}

struct region *
best_fit(size_t size)
{
	size_t current_size = 0;

	struct block *next_block = block_free_list;
	struct region *smallest = NULL;
	while (next_block != NULL) {
		struct region *next_region = next_block->region;
		while (next_region != NULL) {
			if (next_region->free && next_region->size >= size && (current_size == 0 || next_region->size < current_size)) {
				smallest = next_region;
				current_size = next_region->size;
			}
			next_region = next_region->next;
		}
		next_block = next_block->next;
	}

	return smallest;
}


void
initialize_block_list(size_t size)
{
	struct block *block_list =
	        (struct block *) mmap(NULL,
	                              size,
	                              PROT_READ | PROT_WRITE,
	                              MAP_PRIVATE | MAP_ANONYMOUS,
	                              -1,
	                              0);
	block_free_list = block_list;
	block_free_list->next = NULL;
	block_free_list->size = size - sizeof(struct block);
	block_free_list->region = (struct region *) ((char*) block_list + sizeof(struct block));
	block_free_list->region->free = true;
	block_free_list->region->size =
	        size - sizeof(struct block) - sizeof(struct region);
	block_free_list->region->next = NULL;
	if (!print_statistics_set){
		atexit(print_statistics);
		print_statistics_set = true;
	}
}

size_t
get_min_block_size(size_t requested_size)
{
	size_t size =
	        (requested_size + sizeof(struct block) + sizeof(struct region));
	if (size <= MIN_BLOCK)
		return MIN_BLOCK;
	if (size <= MEDIUM_BLOCK)
		return MEDIUM_BLOCK;
	return LARGE_BLOCK;
}

// finds the next free region
// that holds the requested size
//
struct region *
find_free_region(size_t size)
{
	if (block_free_list == NULL) {
		size_t block_size = get_min_block_size(size);
		initialize_block_list(block_size);
	}
#ifdef FIRST_FIT
	return first_fit(size);
#endif

#ifdef BEST_FIT
	return best_fit(size);
#endif
}

struct region *
grow_heap(size_t size)
{
	size_t block_size = get_min_block_size(size);

	size_t total_memory_size = 0;

	struct block *next_block = block_free_list;
	while (next_block->next != NULL) {
		total_memory_size += next_block->size + sizeof(struct block);
		next_block = next_block->next;
	}
	total_memory_size += next_block->size + sizeof(struct block);
	if (total_memory_size + block_size > MAX_ALLOCATION) {
		return NULL;
	}

	struct block *block_list =
	        (struct block *) mmap(NULL,
	                              block_size,
	                              PROT_READ | PROT_WRITE,
	                              MAP_PRIVATE | MAP_ANONYMOUS,
	                              -1,
	                              0);
	next_block->next = block_list;
	block_list->next = NULL;
	block_list->size = block_size - sizeof(struct block);
	block_list->region = (struct region *) ((char*) block_list + sizeof(struct block));
	block_list->region->free = true;
	block_list->region->size =
	        block_size - sizeof(struct block) - sizeof(struct region);
	block_list->region->next = NULL;

	return block_list->region;
}

bool 
is_in_heap (void* ptr) {
	if (ptr == NULL) {
		return false;
	}
	for (struct block *current_block = block_free_list; current_block != NULL; current_block = current_block->next) {
		for (struct region *current_region = current_block->region; current_region != NULL; current_region = current_region->next) {
			if (PTR2REGION(ptr) == current_region && !(current_region->free))
				return true;
		}
	}
	return false;
}

void
print_statistics(void)
{
	printfmt("mallocs:   %d\n", amount_of_mallocs);
	printfmt("frees:     %d\n", amount_of_frees);
	printfmt("requested: %d\n", requested_memory);
}

bool
block_is_empty(struct block *current_block)
{
	return (current_block->region->free &&
	        current_block->region->size == current_block->size -
	                                              sizeof(struct region));
}

void
free_unused_blocks()
{
	if (block_free_list == NULL) {
		return;
	}
	struct block *next_block = block_free_list->next;
	if (block_is_empty(block_free_list)) {
		munmap(block_free_list, block_free_list->size);
		block_free_list = next_block;
		return;
	}
	while (next_block != NULL) {
		if (next_block->next != NULL && block_is_empty(next_block->next)) {
			struct block *next_next_block = next_block->next->next;
			munmap(next_block->next, next_block->next->size);
			next_block->next = next_next_block;
			return;
		}
		next_block = next_block->next;
	}
}

void
split_region(struct region *curr, size_t size)
{
	struct region *new_region =
			(struct region *) ((char *) curr +
								sizeof(struct region) + size);
		new_region->size = curr->size - size - sizeof(struct region);
		new_region->free = true;
		new_region->next = curr->next;
		curr->next = new_region;
		
		curr->size = size;
}

void
coalesce_free_regions()
{
	struct block *next_block = block_free_list;
	while (next_block != NULL) {
		struct region *next = next_block->region;
		while (next != NULL) {
			if (next->free && next->next != NULL && next->next->free) {
				next->size +=
				        next->next->size + sizeof(struct region);
				next->next = next->next->next;
			} else {
				next = next->next;
			}
		}
		next_block = next_block->next;
	}
}