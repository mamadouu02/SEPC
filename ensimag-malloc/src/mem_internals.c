/******************************************************
 * Copyright Grégory Mounié 2018-2022                 *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

unsigned long get_magic(void *ptr, MemKind k) {
    return (knuth_mmix_one_round((unsigned long)ptr) & ~(0b11UL)) | k;
}

void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k)
{
    unsigned long *p = ptr;
    void *user_ptr = p + 2;
    unsigned long user_size = size - 4 * sizeof(unsigned long);
    unsigned long magic = get_magic(ptr, k);

    p[0] = size;
    p[1] = magic;
    p = (unsigned long *)((char *)user_ptr + user_size);
    p[0] = magic;
    p[1] = size;

    return user_ptr;
}

Alloc mark_check_and_get_alloc(void *ptr)
{
    unsigned long *p = (unsigned long *)ptr - 2;
    unsigned long size = p[0];
    unsigned long magic = p[1];
    void *memarea_ptr = p;
    MemKind k = magic & 0b11UL;
    assert(magic == get_magic(memarea_ptr, k));

    unsigned long user_size = size - 4 * sizeof(unsigned long);
    p = (unsigned long *)((char *)ptr + user_size);
    assert(p[0] == magic);
    assert(p[1] == size);

    Alloc a = { memarea_ptr, k, size };
    return a;
}

unsigned long mem_realloc_small()
{
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	    handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long mem_realloc_medium()
{
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert(size == (1UL << indice));
    arena.TZL[indice] = mmap(0,
			     size * 2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	    handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}

// used for test in buddy algo
unsigned int nb_TZL_entries()
{
    int nb = 0;
    
    for (int i = 0; i < TZL_SIZE; i++)
        if (arena.TZL[i])
            nb++;

    return nb;
}

void *pop(int i) {
    void **stack = (i < 0) ? &arena.chunkpool : &arena.TZL[i];
    assert(*stack);
    void **ptr = *stack;
    *stack = *ptr;
    return ptr;
}

void push(void** ptr, int i) {
    void **stack = (i < 0) ? &arena.chunkpool : &arena.TZL[i];
    *ptr = *stack;
    *stack = ptr;
}
