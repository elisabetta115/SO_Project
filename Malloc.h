#pragma once
#include <stddef.h>

void *pseudo_malloc(size_t size);
int pseudo_free(void *ptr);
int init_buddy_allocator();
int destroy_buddy_allocator();
int print_buddy_allocator(void *ptr);
int get_bitmap(int index);

#define PAGE_SIZE 4096
#define BUDDY_MEMORY_SIZE (1 << 20)     // 1 MB
#define MIN_BLOCK_SIZE (PAGE_SIZE >> 4) // 1/16 of page size (256 bytes)
#define MAX_LEVELS 16                   // 1 MB / 64 = 16384 blocks (2^14 blocks), log2(16384) = 14 + 1 for initial split

typedef enum
{
    false,
    true
} bool;