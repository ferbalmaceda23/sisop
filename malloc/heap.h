#ifndef _HEAP_H_
#define _HEAP_H_
#define ALIGN4(s) (((((s) -1) >> 2) << 2) + 4)
#define REGION2PTR(r) ((r) + 1)
#define PTR2REGION(ptr) ((struct region *) (ptr) -1)
#define MIN_REQUEST 32
#define MAX_ALLOCATION 41952000
#define MIN_BLOCK 2046
#define MEDIUM_BLOCK 131100
#define LARGE_BLOCK 4195200

struct region {
	bool free;
	size_t size;
	struct region *next;
};

struct block {
	size_t size;
	struct block *next;
	struct region *region;
};

void
initialize_block_list(size_t size);

size_t
get_min_block_size(size_t requested_size);

struct region *
find_free_region(size_t size);

struct region *
grow_heap(size_t size);

bool 
is_in_heap (void* ptr);

struct region *
best_fit(size_t size);

struct region *
first_fit(size_t size);

bool
block_is_empty(struct block *current_block);

void
free_unused_blocks(void);

void
print_statistics(void);

void
split_region(struct region *curr, size_t size);

void
coalesce_free_regions(void);

#endif