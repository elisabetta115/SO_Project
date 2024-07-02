#include <stddef.h>

void *pseudo_malloc(size_t size);
int pseudo_free(void *ptr);
void init_buddy_allocator();
void destroy_buddy_allocator();
void print_buddy_allocator(void *ptr);

