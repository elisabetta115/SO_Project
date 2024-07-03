#include "malloc.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#define PAGE_SIZE 4096
#define BUDDY_MEMORY_SIZE (1 << 20) // 1 MB
#define MIN_BLOCK_SIZE (PAGE_SIZE >> 4) // 1/16 of page size
#define MAX_LEVELS 16 // 1 MB / 64 = 16384 blocks (2^14 blocks), log2(16384) = 14 + 1 for initial split

//An array of bytes used as a bitmap to track the allocation status of each block of memory.
static unsigned char buddy_bitmap[BUDDY_MEMORY_SIZE / MIN_BLOCK_SIZE / 8];
//Pointer to the start of the allocated memory region
static void *buddy_memory;
static int level_size[MAX_LEVELS];

/*HELPER FUNCTIONS FOR BUDDY ALLOCATOR*/

// Helper function to get buddy index
int get_buddy_index(size_t size) {
    int index = 0;
    // Find the smallest power of 2 that is greater than the requested size
    while ((1 << index) * MIN_BLOCK_SIZE < size) {
        index++;
    }
    return index;
}

// Helper function to set buddy bitmap
void set_buddy_bitmap(int index, int value) {
    // Calculate the byte and bit position in the bitmap
    int byte = index / 8;
    int bit = index % 8;
    if (value) {
        // Create a mask with the bit position set to 1
        //Perform a bitwise OR operation to set the bit to 1
        buddy_bitmap[byte] |= (1 << bit);
    } else {
        // Create a mask with the bit position set to 0
        //Perform a bitwise AND operation to set the bit to 0
        buddy_bitmap[byte] &= ~(1 << bit);
    }
}

// Helper function to find free buddy block
int find_free_buddy(int index) {
    // i is the index of the first block at the level 
    // sizeof(buddy_bitmap) * 8 is the total number of bit in the bitmap
    for (int i = index; i < sizeof(buddy_bitmap) * 8; i++) {
        // i / 8 determinate the byte position in the bitmap
        // (1 << (i % 8)) set a mask with the bit at position i%8 set to 1
        if ((buddy_bitmap[i / 8] & (1 << (i % 8))) == 0) {
            return i;
        }
    }
    return -1;
}

// Helper function to split buddy block
void split_buddy(int index) {
    //index represents the block that needs to be split to accommodate a memory allocation.
    //buddy_bitmap[index / 2] == 0 the buddy block is free 
    while (index > 0 && buddy_bitmap[index / 2] == 0) {
        index /= 2;
        //Marks the current block (index) as allocated by setting its bitmap to 1.
        set_buddy_bitmap(index, 1);
        //Marks the left buddy block as free by setting its bitmap to 0.
        set_buddy_bitmap(index * 2, 0);
        //Marks the right buddy block as free by setting its bitmap to 0.
        set_buddy_bitmap(index * 2 + 1, 0);
    }
}

// Helper function to merge buddy blocks
void merge_buddy(int index) {
    while (index > 0) {
        //The buddy index is calculated based on whether the current index is even or odd.
        int buddy_index = index % 2 == 0 ? index + 1 : index - 1;
        //& isolates the specific bit within the byte of buddy_bitmap corresponding to buddy_index
        if (buddy_bitmap[buddy_index / 8] & (1 << (buddy_index % 8))) {
            break;
        }
        //Marks the parent block as free
        set_buddy_bitmap(index / 2, 0);
        index /= 2;
    }
}

// Function to print the buddy allocator
void print_buddy_allocator(void *ptr) {
    printf("\n\n\nBuddy Allocator State:\n");
    printf("Block Size: %d bytes\n", MIN_BLOCK_SIZE);
    printf("Total Blocks: %d\n", BUDDY_MEMORY_SIZE / MIN_BLOCK_SIZE);
    printf("Bitmap State (1: Allocated, 0: Free):\n");
    
    int total_blocks = BUDDY_MEMORY_SIZE / MIN_BLOCK_SIZE;
    int target_index = -1;
    if (ptr != NULL) {
        target_index = (ptr - buddy_memory) / MIN_BLOCK_SIZE;
    }
    /*TODO: handle case ptr not in buddy allocator*/
    /*TODO: better print function*/
    for (int i = 0; i < total_blocks; i++) {
        int byte = i / 8;
        int bit = i % 8;
        int allocated = (buddy_bitmap[byte] & (1 << bit)) != 0;
        if (i == target_index) {
            printf("[%d]", allocated); // Highlight the block corresponding to ptr
        } else {
            printf("%d", allocated);
        }
        if ((i + 1) % 64 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}


/* MALLOC FUNCTIONS*/

// Buddy allocator function
void *buddy_alloc(size_t size) {
    //Determines the appropriate index in the buddy system for the requested size
    int index = get_buddy_index(size);
    int free_index = find_free_buddy(index);
    if (free_index == -1) {
        return NULL;
    }
    split_buddy(free_index);
    set_buddy_bitmap(free_index, 1);
    //Return a pointer to the allocated memory block
    //buddy memory is the start of the allocated memory region
    //free_index * MIN_BLOCK_SIZE results in the exact byte address where the allocated block starts
    return buddy_memory + free_index * MIN_BLOCK_SIZE;
}

// Large allocation function
void *large_alloc(size_t size) {
    // Calculate the total size including space to store the allocation size
    size_t total_size = size + sizeof(size_t);
    void *ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    // Store the total size at the beginning of the allocated block
    *((size_t *)ptr) = total_size;
    // Return a pointer to the usable memory (after the size)
    return (char *)ptr + sizeof(size_t);
}


// Custom malloc function
void *pseudo_malloc(size_t size) {
    if(size <= 0) {
        errno = EINVAL;
        return NULL;
    }
    if (size < PAGE_SIZE / 4) {
        return buddy_alloc(size);
    } else {
        return large_alloc(size);
    }
}


/*FREE FUNCTION*/

// Large free function
int large_free(void *ptr) {
    if (ptr == NULL) {
        errno = EINVAL;
        return -1;
    }
    // Get the original pointer by subtracting the size of size_t
    void *real_ptr = (char *)ptr - sizeof(size_t);
    // Retrieve the total size stored at the beginning of the block
    size_t size = *((size_t *)real_ptr);
    if(munmap(real_ptr, size) == -1){
        errno = EINVAL;
        return -1;
    }
}

// Buddy free function
int buddy_free(void *ptr) {
    /*TODO: check block already free*/
    //Calculate the index of the block in the buddy system
    int index = (ptr - buddy_memory) / MIN_BLOCK_SIZE;
    set_buddy_bitmap(index, 0);
    merge_buddy(index);
}

// Custom free function
int pseudo_free(void *ptr) {
    if(ptr == NULL) {
        return -1;
    }
    if (ptr >= buddy_memory && ptr < buddy_memory + BUDDY_MEMORY_SIZE) {
        buddy_free(ptr);
    } else {
        large_free(ptr);
    }
    return 1;
}

/*BUDDY_MEMORY*/

// Constructor function to initialize buddy allocator
int init_buddy_allocator() {
    buddy_memory = mmap(NULL, BUDDY_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (buddy_memory == MAP_FAILED) {
        return(-1);
    }
    memset(buddy_bitmap, 0, sizeof(buddy_bitmap));
}

// Destructor function to destroy buddy allocator
int destroy_buddy_allocator() {
    // Unmap the buddy memory
    if (munmap(buddy_memory, BUDDY_MEMORY_SIZE) == -1) {
       return(-1);
    }
    // Clear the buddy_bitmap array
    memset(buddy_bitmap, 0, sizeof(buddy_bitmap));
}