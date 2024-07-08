#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "Malloc.h"

#define DEBUG

// An array of bytes used as a bitmap to track the allocation status of each block of memory.
static unsigned char buddy_bitmap[BUDDY_MEMORY_SIZE / MIN_BLOCK_SIZE / 8] = {0};
// Pointer to the start of the allocated memory region
static void *buddy_memory;

/*HELPER FUNCTIONS FOR BUDDY ALLOCATOR*/

// Helper function to check if the bitmap is full
int is_bitmap_full()
{
    size_t bitmap_bytes = sizeof(buddy_bitmap);
    for (size_t i = 0; i < bitmap_bytes; i++)
    {
        for (int bit = 0; bit < 8; bit++)
        {
            if (!(get_bitmap(i) & (1 << bit)))
            {
                return false;
            }
        }
    }
    return true;
}


// Helper function to get buddy index
int get_buddy_index(size_t size)
{
    int index = 0;
    // Find the smallest power of 2 that is greater than the requested size
    while ((1 << index) * MIN_BLOCK_SIZE < (int)size)
    {
        index++;
    }
    return index;
}

// Helper function to set buddy bitmap
void set_bitmap(int index, int value)
{
    // Calculate the byte and bit position in the bitmap
    int byte = index / 8;
    int bit = index % 8;
    if (value)
    {
        // Create a mask with the bit position set to 1
        // Perform a bitwise OR operation to set the bit to 1
        buddy_bitmap[byte] |= (1 << bit);
    }
    else
    {
        // Create a mask with the bit position set to 0
        // Perform a bitwise AND operation to set the bit to 0
        buddy_bitmap[byte] &= ~(1 << bit);
    }
}

// Get the buddy bitmap
int get_bitmap(int index)
{
    int byte = index / 8;
    int bit = index % 8;
    return (buddy_bitmap[byte] & (1 << bit)) != 0;
}

// Helper function to find free buddy block
int find_free_buddy(int index)
{
    for (int i = index; i < (int)sizeof(buddy_bitmap) * 8; i++)
    {
        if (get_bitmap(i) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Helper function to split buddy block
void split_buddy(int index)
{
    int parent_index;
    while (index > 0 && get_bitmap(index / 2) == 0)
    {
        parent_index = index / 2;
        set_bitmap(parent_index, 1);         // Mark parent as allocated
        set_bitmap(parent_index * 2, 0);     // Mark left child as free
        set_bitmap(parent_index * 2 + 1, 0); // Mark right child as free
        index = parent_index;
    }
}

#ifdef DEBUG
void print_bitmap()
{
    for (int i = 0; i < (int)sizeof(buddy_bitmap); i++)
    {
        printf("%d", get_bitmap(i));
    }
    printf("\n");
}
#endif

/* MALLOC FUNCTIONS*/

// Large allocation function
void *large_alloc(size_t size)
{
    // Calculate the total size including space to store the allocation size
    size_t total_size = size + sizeof(size_t);
    void *ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED)
    {
        errno = EINVAL;
        return NULL;
    }
    // Store the total size at the beginning of the allocated block
    *((size_t *)ptr) = total_size;
    // Return a pointer to the usable memory (after the size)
    return (char *)ptr + sizeof(size_t);
}

// Buddy allocator function
void *buddy_alloc(size_t size)
{
    int index = get_buddy_index(size);
    int free_index = find_free_buddy(index);

    if (free_index == -1)
    {
        // Fall back on large allocation if no free block is found
        return large_alloc(size);
    }

    split_buddy(free_index); // Split the block if necessary
    set_bitmap(free_index, 1);
    return buddy_memory + free_index * MIN_BLOCK_SIZE;
}

// Custom malloc function
void *pseudo_malloc(size_t size)
{
    if (size <= 0)
    {
        errno = EINVAL;
        return NULL;
    }
    else if (size < PAGE_SIZE / 4)
    {
        return buddy_alloc(size);
    }
    else
    {
        return large_alloc(size);
    }
}

/*FREE FUNCTION*/

// Large free function
int large_free(void *ptr)
{
    if (ptr == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    // Get the original pointer by subtracting the size of size_t
    void *real_ptr = (char *)ptr - sizeof(size_t);
    // Retrieve the total size stored at the beginning of the block
    size_t size = *((size_t *)real_ptr);
    if (munmap(real_ptr, size) == -1)
    {
        errno = EINVAL;
        return -1;
    }
    return 1;
}

// Buddy free function
int buddy_free(void *ptr)
{
    int index = (ptr - buddy_memory) / MIN_BLOCK_SIZE;
    if (get_bitmap(index) == 0)
    {
        errno = EINVAL;
        return -1;
    }
    set_bitmap(index, 0);      
    return 0;
}

// Custom free function
int pseudo_free(void *ptr)
{
    int ret = 1;
    if (ptr == NULL)
    {
        return -1;
    }
    if (ptr >= buddy_memory && ptr <= (buddy_memory + BUDDY_MEMORY_SIZE))
    {
        if (buddy_free(ptr) == -1)
        {
            ret = -1;
        }
    }
    else
    {
        if (large_free(ptr) == -1)
        {
            ret = -1;
        }
    }
    return ret;
}

/*BUDDY_MEMORY*/

// Constructor function to initialize buddy allocator
int init_buddy_allocator()
{
    buddy_memory = mmap(NULL, BUDDY_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (buddy_memory == MAP_FAILED)
    {
        return (-1);
    }
    memset(buddy_bitmap, 0, sizeof(buddy_bitmap));
    return 0;
}

// Destructor function to destroy buddy allocator
int destroy_buddy_allocator()
{
    // Unmap the buddy memory
    if (munmap(buddy_memory, BUDDY_MEMORY_SIZE) == -1)
    {
        return (-1);
    }
    // Clear the buddy_bitmap array
    memset(buddy_bitmap, 0, sizeof(buddy_bitmap));
    return 0;
}
