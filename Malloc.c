#include "malloc.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096
#define BUDDY_MEMORY_SIZE (1 << 20) // 1 MB
#define MIN_BLOCK_SIZE (PAGE_SIZE >> 4) // 1/16 of page size
#define MAX_LEVELS 16 // 1 MB / 64 = 16384 blocks (2^14 blocks), log2(16384) = 14 + 1 for initial split

static unsigned char buddy_bitmap[BUDDY_MEMORY_SIZE / MIN_BLOCK_SIZE / 8];
static void *buddy_memory;
static int level_size[MAX_LEVELS];

// Helper function to get buddy index
int get_buddy_index(size_t size) {
    int index = 0;
    while ((1 << index) * MIN_BLOCK_SIZE < size) {
        index++;
    }
    return index;
}

// Helper function to set buddy bitmap
void set_buddy_bitmap(int index, int value) {
    int byte = index / 8;
    int bit = index % 8;
    if (value) {
        buddy_bitmap[byte] |= (1 << bit);
    } else {
        buddy_bitmap[byte] &= ~(1 << bit);
    }
}

// Helper function to find free buddy block
int find_free_buddy(int index) {
    for (int i = index; i < sizeof(buddy_bitmap) * 8; i++) {
        if ((buddy_bitmap[i / 8] & (1 << (i % 8))) == 0) {
            return i;
        }
    }
    return -1;
}

// Helper function to split buddy block
void split_buddy(int index) {
    while (index > 0 && buddy_bitmap[index / 2] == 0) {
        index /= 2;
        set_buddy_bitmap(index, 1);
        set_buddy_bitmap(index * 2, 0);
        set_buddy_bitmap(index * 2 + 1, 0);
    }
}

// Buddy allocator function
void *buddy_alloc(size_t size) {
    int index = get_buddy_index(size);
    int free_index = find_free_buddy(index);
    if (free_index == -1) {
        return NULL;
    }
    split_buddy(free_index);
    set_buddy_bitmap(free_index, 1);
    return buddy_memory + free_index * MIN_BLOCK_SIZE;
}

// Helper function to merge buddy blocks
void merge_buddy(int index) {
    while (index > 0) {
        int buddy_index = index % 2 == 0 ? index + 1 : index - 1;
        if (buddy_bitmap[buddy_index / 8] & (1 << (buddy_index % 8))) {
            break;
        }
        set_buddy_bitmap(index / 2, 0);
        index /= 2;
    }
}

// Buddy free function
void buddy_free(void *ptr) {
    int index = (ptr - buddy_memory) / MIN_BLOCK_SIZE;
    set_buddy_bitmap(index, 0);
    merge_buddy(index);
}


// Custom malloc function
void *pseudo_malloc(size_t size) {
    if (size < PAGE_SIZE / 4) {
        return buddy_alloc(size);
    } else {
        //return large_alloc(size);
    }
}

// Custom free function
void pseudo_free(void *ptr) {
    if (ptr >= buddy_memory && ptr < buddy_memory + BUDDY_MEMORY_SIZE) {
        buddy_free(ptr);
    } else {
        //large_free(ptr);
    }
}

// Constructor function to initialize buddy allocator
void init_buddy_allocator() {
    buddy_memory = mmap(NULL, BUDDY_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (buddy_memory == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    memset(buddy_bitmap, 0, sizeof(buddy_bitmap));
}