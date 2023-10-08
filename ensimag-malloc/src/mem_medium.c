/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

unsigned int puiss2(unsigned long size) {
    unsigned int p = 0;
    size = size - 1; // allocation start in 0

    while (size) { // get the largest bit
        p++;
        size >>= 1;
    }

    if (size > (1 << p))
	    p++;
    
    return p;
}

void *get_buddy_ptr(void *ptr, unsigned long size)
{
    return (void *)((unsigned long)ptr ^ size);
}

void *emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);

    unsigned int i = puiss2(size + 4 * sizeof(unsigned long));
    unsigned int j = i;

    while (j < FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant && arena.TZL[j] == NULL) {
        j++;
    }

    if (j == FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant) {
        mem_realloc_medium();
    }

    void *ptr = pop(j);
    
    while (j > i) {
        void *buddy_ptr = get_buddy_ptr(ptr, 1 << --j);
        push(buddy_ptr, j);
    }
    
    return mark_memarea_and_get_user_ptr(ptr, 1 << i, MEDIUM_KIND);
}

void efree_medium(Alloc a) {
    unsigned long i = puiss2(a.size);
    void *ptr = a.ptr;
    void *buddy_ptr = get_buddy_ptr(ptr, a.size);
    void **pp = NULL;
    void **p = arena.TZL[i];

    while (p) {
        if (p == buddy_ptr) {
            if (pp) {
                *pp = *p;
            } else {
                arena.TZL[i] = *p;
            }

            ptr = ((unsigned long)ptr < (unsigned long)buddy_ptr) ? ptr : buddy_ptr;
            buddy_ptr = get_buddy_ptr(ptr, 1 << ++i);
            pp = NULL;
            p = arena.TZL[i];
            continue;
        }

        pp = p;
        p = *p;
    }

    push(ptr, i);
}
