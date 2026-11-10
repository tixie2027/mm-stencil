#include "buddy_allocator.h"

#include "sys/mem/mem.h"

#include "util/ktypes.h"
#include "util/string.h"

/* helpers! */
static chunk_t *heads[5];

/* smallest order whose block size (PAGE_SIZE << order) holds size bytes. */
static int order_for(long size) {
    int order = 0;
    while ((PAGE_SIZE << order) < size) {
        order++;
    }
    return order;
}

/* push the free block at addr onto heads[order]. */
static void push(int order, void *addr) {
    chunk_t *chunk = (chunk_t *)addr;

    chunk->addr = addr;
    chunk->next = heads[order];
    heads[order] = chunk;
}

/* check if the block at addr is on heads[order] */
static int take(int order, void *addr) {
    chunk_t **link = &heads[order];

    while (*link != NULL) {
        if ((*link)->addr == addr) {
            *link = (*link)->next;
            return 1;
        }
        link = &(*link)->next;
    }

    return 0;
}

/* reserve metadata, then populate the free lists */
void buddy_init() {
    long pages = mem_size / PAGE_SIZE;
    long page = 0;

    for (int order = 4; order >= 0; order--) {
        long block_pages = 1L << order;

        heads[order] = NULL;

        while (block_pages <= pages - page) {
            push(order, (char *)dram + page * PAGE_SIZE);
            page += block_pages;
        }
    }
}

/*
 *   - given a size, how do we know where to allocate from?
 *   - if we don't have any available chunks, how do we find another one?
 */
void *buddy_obj_alloc(long size) {
    if (size <= 0 || size > MAX_CHUNK_SIZE) {
        return NULL;
    }

    int want = order_for(size);
    int order = want;

    /* Find the smallest non-empty list */
    while (order < 5 && heads[order] == NULL) {
        order++;
    }

    if (order == 5) {
        return NULL;
    }

    /* Pop the head of list. */
    char *block = (char *)heads[order];
    heads[order] = heads[order]->next;

    /* freeing the upper half each time */
    while (order > want) {
        order--;
        push(order, block + (PAGE_SIZE << order));
    }

    return block;
}

/*
 * Hint: this function should handle the inverse cases of the "alloc" function!
 *       if there are cases that need to be handled in that function, what
 *       would the inverse cases look like?
 */
void buddy_obj_free(long size, void *obj) {
    if (obj == NULL || size <= 0 || size > MAX_CHUNK_SIZE) {
        return;
    }

    int order = order_for(size);
    long page = ((char *)obj - (char *)dram) / PAGE_SIZE;

    /* Merge as long as the buddy is also free. */
    while (order < 4) {
        long buddy = page ^ (1L << order);
        void *buddy_addr = (char *)dram + buddy * PAGE_SIZE;

        if (!take(order, buddy_addr)) {
            break;
        }

        if (buddy < page) {
            page = buddy;
        }

        order++;
    }

    push(order, (char *)dram + page * PAGE_SIZE);
}

/*
 * TODO: implement me!
 *
 * Hint: this function behaves similar to "obj_alloc" but what happens if
 *       npages is larger than MAX_CHUNK_SIZE?
 */
void *page_alloc_n(int npages) {
    return NULL;
}

/*
 * TODO: implement me!
 *
 * Hint: this function should handle the inverse case of the "alloc" function!
 */
void page_free_n(void *ptr, int npages) {

}
