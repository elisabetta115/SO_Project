#include "malloc.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define BUDDY_MEMORY_SIZE (1 << 20) // 1 MB

int testsRun = 0;
int testsPassed = 0;

typedef enum
{
    false,
    true
} bool;

void printTest(int condition, const char *message)
{
    testsRun++;
    if (condition)
    {
        testsPassed++;
        printf("Test %d\t passed:\t %s\n", testsRun, message);
    }
    else
    {
        printf("Test %d\t failed:\t %s\n", testsRun, message);
    }
}

void test_small_allocation()
{
    void *ptr;
    int i = print_buddy_allocator(ptr);
    ptr = pseudo_malloc(100);
    printTest(ptr != NULL, "Small allocation");
    i = print_buddy_allocator(ptr);
    pseudo_free(ptr);
}

void test_large_allocation()
{
    void *ptr = pseudo_malloc(5000);
    printTest(ptr != NULL, "Large allocation");
    pseudo_free(ptr);
}

void test_memory_reuse()
{
    void *ptr1 = pseudo_malloc(100);
    pseudo_free(ptr1);
    void *ptr2 = pseudo_malloc(100);
    printTest(ptr1 == ptr2, "Memory reuse");
    pseudo_free(ptr2);
}

void test_boundary_conditions()
{
    void *ptr = pseudo_malloc(PAGE_SIZE / 4 - 1);
    printTest(ptr != NULL, "Boundary condition");
    pseudo_free(ptr);
}

void test_multiple_allocations()
{
    void *ptrs[10];
    bool passed = true;
    for (int i = 0; i < 10; i++)
    {
        ptrs[i] = pseudo_malloc(100);
        if (ptrs[i] == NULL)
        {
            passed = false;
        }
    }

    printTest(passed, "Multiple allocations");
    for (int i = 0; i < 10; i++)
    {
        pseudo_free(ptrs[i]);
    }
}

void test_write_and_read()
{
    void * p = pseudo_malloc(16);
    void *ptr1 = pseudo_malloc(16);
    pseudo_free(p);
    void *ptr2 = pseudo_malloc(64);
    printf("ptr1: %d, ptr2: %d\n", ptr1, ptr2);
    const char *test_string2 = "Hello, World!Hello, World!Hello, World!Hello, World!";
    const char *test_string1 = "Hello, World!";
    size_t len1 = strlen(test_string1) + 1;
    size_t len2 = strlen(test_string2) + 1;
    ptr1 = pseudo_malloc(len1);
    ptr2 = pseudo_malloc(len2);
    printTest(ptr1 != NULL && ptr2 != NULL, "Write and read allocation");

    strcpy(ptr1, test_string1);
    strcpy(ptr2, test_string2);

    char *read_string1 = (char *)ptr1;
    char *read_string2 = (char *)ptr2;

    printf("read_string1: %s\n", read_string1);
    printf("read_string2: %s\n", read_string2);

    printTest(strcmp(read_string1, test_string1) == 0 && strcmp(read_string2, test_string2) == 0, "Write and read");

    pseudo_free(ptr1);
    pseudo_free(ptr2);
}

void test_large_allocation_boundary()
{
    void *ptr = pseudo_malloc(PAGE_SIZE / 4);
    printTest(ptr != NULL, "Large allocation boundary");
    pseudo_free(ptr);
}

void test_small_allocation_fill()
{
    void *ptrs[PAGE_SIZE / 4];
    bool passed = true;
    for (int i = 0; i < PAGE_SIZE / 4; i++)
    {
        ptrs[i] = pseudo_malloc(1);
        if (ptrs[i] == NULL)
        {
            passed = false;
        }
    }
    printTest(passed, "Small allocation fill");
    for (int i = 0; i < PAGE_SIZE / 4; i++)
    {
        pseudo_free(ptrs[i]);
    }
}

void test_stress_small_allocations()
{
    bool passed = true;
    for (int i = 0; i < 10000; i++)
    {
        void *ptr = pseudo_malloc(1);
        if (ptr == NULL)
        {
            passed = false;
            break;
        }
        pseudo_free(ptr);
    }
    printTest(passed, "Stress small allocations");
}

void test_stress_large_allocations()
{
    bool passed = true;
    for (int i = 0; i < 1000; i++)
    {
        void *ptr = pseudo_malloc(PAGE_SIZE);
        if (ptr == NULL)
        {
            passed = false;
            break;
        }
        pseudo_free(ptr);
    }
    printTest(passed, "Stress large allocations");
}

void test_fragmentation()
{
    void *ptrs[100];
    for (int i = 0; i < 100; i++)
    {
        ptrs[i] = pseudo_malloc(50);
    }
    for (int i = 0; i < 100; i += 2)
    {
        pseudo_free(ptrs[i]);
    }
    bool passed = true;
    for (int i = 0; i < 50; i++)
    {
        void *ptr = pseudo_malloc(50);
        if (ptr == NULL)
        {
            passed = false;
        }
    }
    for (int i = 1; i < 100; i += 2)
    {
        pseudo_free(ptrs[i]);
    }

    printTest(passed, "Fragmentation test");
}

void test_edge_case_zero_allocation()
{
    void *ptr = pseudo_malloc(0);
    printTest(ptr != NULL, "Edge case zero allocation");
    pseudo_free(ptr);
}

void test_edge_case_large_allocation()
{
    void *ptr = pseudo_malloc(BUDDY_MEMORY_SIZE);
    printTest(ptr != NULL, "Edge case large allocation");
    pseudo_free(ptr);
}

void test_small_allocation_and_free()
{
    bool passed = true;
    for (int i = 0; i < 100; i++)
    {
        void *ptr = pseudo_malloc(100);
        if (ptr == NULL)
        {
            passed = false;
            break;
        }
        pseudo_free(ptr);
    }
    printTest(passed, "Small allocation and free");
}

void test_large_allocation_and_free()
{
    bool passed = true;
    for (int i = 0; i < 10; i++)
    {
        void *ptr = pseudo_malloc(5000);
        if (ptr == NULL)
        {
            passed = false;
            break;
        }
        pseudo_free(ptr);
    }
    printTest(passed, "Large allocation and free");
}

void test_random_allocations()
{
    bool passed = true;
    for (int i = 0; i < 100; i++)
    {
        size_t size = rand() % 10000;
        void *ptr = pseudo_malloc(size);
        if (ptr == NULL)
        {
            passed = false;
            pseudo_free(ptr);
            break;
        }
        pseudo_free(ptr);
    }
    printTest(passed, "Random allocations");
}

void test_pointer_distance(){
    void *first_location = pseudo_malloc(1);
    pseudo_free(first_location);
    void *ptr = pseudo_malloc(16);
    printf("ptr: %d, first_location: %d\n", ptr, first_location);
    printTest(first_location == ptr, "Same pointer");
    ptr = pseudo_malloc(16);
    printf("ptr: %d, first_location: %d\n", ptr, first_location);
    printTest(ptr-first_location == 16, "Pointer distance");
    pseudo_free(first_location);
    ptr = pseudo_malloc(64);
    printf("ptr: %d, first_location: %d\n", ptr, first_location);
    printTest(ptr-first_location == 64, "Pointer distance 2");
    pseudo_free(first_location+16);
    pseudo_free(ptr);
    printf("ptr: %d, first_location: %d\n", ptr, first_location);
}

int main()
{
    printf("Starting testing...\n\n\n");
    clock_t beginningTime = clock();

    int ret = init_buddy_allocator();
    if (ret == -1)
    {
        printf("Failed to initialize buddy allocator\n");
        return -1;
    }

    // Run all tests
    test_small_allocation();
    test_large_allocation();
    test_memory_reuse();
    test_boundary_conditions();
    test_multiple_allocations();
    test_write_and_read();
    test_large_allocation_boundary();
    test_small_allocation_fill();
    test_stress_small_allocations();
    test_stress_large_allocations();
    test_fragmentation();
    test_edge_case_zero_allocation();
    test_edge_case_large_allocation();
    test_small_allocation_and_free();
    test_large_allocation_and_free();
    test_random_allocations();
    test_pointer_distance();
    /*TODO: Add test with write and read from a file*/
    /*TODO: move the free functions in separate loop */

    ret = destroy_buddy_allocator();
    if (ret == -1)
    {
        printf("Failed to destroy buddy allocator\n");
        return -1;
    }

    clock_t endingTime = clock();

    printf("\n\nTotal tests ran:\t%d\n", testsRun);
    printf("Total tests passed:\t%d\n\n\n", testsPassed);
    printf("Ended testing.\n\n\n");
    printf("Time taken: %f\n", (double)(endingTime - beginningTime) / CLOCKS_PER_SEC);

    return 0;
}
