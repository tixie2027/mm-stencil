#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "buddy_allocator.h"

int main(void) {
    mem_size = 1024 * 1024;
    dram = malloc(mem_size);
    assert(dram);
    buddy_init();

    void *first = buddy_obj_alloc(PAGE_SIZE);
    assert(first != NULL);

    while (buddy_obj_alloc(PAGE_SIZE) != NULL) {}
    assert(buddy_obj_alloc(PAGE_SIZE) == NULL);

    buddy_obj_free(PAGE_SIZE, first);

    void *second = buddy_obj_alloc(PAGE_SIZE);
    assert(second != NULL);
    
    puts("PASS: allocate, free, allocate again");
    free(dram);
    return 0;
}