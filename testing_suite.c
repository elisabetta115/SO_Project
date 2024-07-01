#include "malloc.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>

#define PAGE_SIZE 4096

#define TEST_PASS 1
#define TEST_FAIL 0

int testsRun = 0;
int testsPassed = 0;

void asserTest(int condition, const char* message) {
    testsRun++;
    if (condition) {
        testsPassed++;
        printf("Test %d passed: %s\n", testsRun, message);
    } else {
        printf("Test %d failed: %s\n", testsRun, message);
    }
}


void test_buddy_allocator() {

    init_buddy_allocator();

    // Test small allocation
    void *ptr = pseudo_malloc(100);
    asserTest(ptr != NULL, "Small allocation");

    // Test write and read
    const char *test_string = "Hello World!";
    size_t len = strlen(test_string) + 1;

    // Write to memory
    strcpy(ptr, test_string);

    // Read from memory and verify
    char *read_string = (char *)ptr;
    asserTest(strcmp(read_string, test_string) == 0, "Write and read");

    pseudo_free(ptr);
    printf("\n\nTotal tests ran:\t%d\n", testsRun);
	printf("Total tests passed:\t%d\n\n\n", testsPassed);
}

int main() {

    printf("Starting testing...\n\n\n");
    clock_t beginningTime = clock();
    test_buddy_allocator();
    clock_t endingTime = clock();
	printf("Ended testing.\n\n\n");
    printf("Time taken: %f\n", (double)(endingTime - beginningTime) / CLOCKS_PER_SEC);

    return 0;
}
