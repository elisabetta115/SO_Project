#include "malloc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define PAGE_SIZE 4096

#define TEST_PASS 1
#define TEST_FAIL 0

int testsRun = 0;
int testsPassed = 0;

// Test cases for the custom malloc implementation
void test_small_allocation() {
    void *ptr = pseudo_malloc(100);
    assert(ptr != NULL);
    printf("Test small allocation passed: %p\n", ptr);
    pseudo_free(ptr);
}



void test_write_and_read() {
    const char *test_string = "Hello World!";
    size_t len = strlen(test_string) + 1;

    void *ptr = pseudo_malloc(len);
    assert(ptr != NULL);

    // Write to memory
    strcpy(ptr, test_string);

    // Read from memory and verify
    char *read_string = (char *)ptr;
    printf("Test write and read passed: %s\n", read_string);
    assert(strcmp(read_string, test_string) == 0);

    pseudo_free(ptr);
}

int main() {

    printf("Starting testing...\n");


    init_buddy_allocator();

    // Run test cases
    test_small_allocation();
    test_write_and_read();

    return 0;
}
