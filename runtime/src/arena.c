#include "elx_runtime.h"
#include <stdlib.h>
#include <string.h>

bool elx_arena_create(elx_arena_t *arena, size_t initial_size) {
    if (!arena) return false;
    if (initial_size == 0U) initial_size = ELX_ARENA_BLOCK_SIZE;
    arena->base = (uint8_t *)malloc(initial_size);
    if (!arena->base) return false;
    memset(arena->base, 0, initial_size);
    arena->data = arena->base;
    arena->used = 0U;
    arena->capacity = initial_size;
    return true;
}

void elx_arena_destroy(elx_arena_t *arena) {
    if (!arena) return;
    free(arena->base);
    arena->base = NULL;
    arena->data = NULL;
    arena->used = 0U;
    arena->capacity = 0U;
}

void *elx_arena_alloc(elx_arena_t *arena, size_t size) {
    uint8_t *ptr;
    if (!arena || !arena->base || size == 0U) return NULL;
    if (arena->used + size > arena->capacity) {
        size_t new_capacity = arena->capacity * 2U;
        size_t total_needed = arena->used + size;
        while (new_capacity < total_needed) new_capacity *= 2U;
        uint8_t *new_base = (uint8_t *)realloc(arena->base, new_capacity);
        if (!new_base) return NULL;
        memset(new_base + arena->capacity, 0, new_capacity - arena->capacity);
        arena->base = new_base;
        arena->data = arena->base + arena->used;
        arena->capacity = new_capacity;
    }
    ptr = arena->data;
    arena->data += size;
    arena->used += size;
    return ptr;
}

void elx_arena_reset(elx_arena_t *arena) {
    if (!arena || !arena->base) return;
    memset(arena->base, 0, arena->capacity);
    arena->data = arena->base;
    arena->used = 0U;
}
