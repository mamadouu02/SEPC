/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

void *emalloc_small(unsigned long size)
{
    if (arena.chunkpool == NULL) {
        unsigned long nb_chunks = mem_realloc_small() / CHUNKSIZE;
        char **ptr = arena.chunkpool;

        for (int i = 0; i < nb_chunks - 1; i++) {
            ptr[96 * i] = (void *)(ptr + CHUNKSIZE * (i + 1));
        }

        ptr[96 * (nb_chunks - 1)] = NULL;
    }

    void **ptr = arena.chunkpool;
    arena.chunkpool = *ptr;

    return mark_memarea_and_get_user_ptr(ptr, CHUNKSIZE, SMALL_KIND);
}

void efree_small(Alloc a)
{
    *((char *)a.ptr) = arena.chunkpool;
    arena.chunkpool = a.ptr;
}
