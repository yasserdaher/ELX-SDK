#include "elx_runtime.h"
#include <stdlib.h>
#include <string.h>

void elx_stack_init(elx_stack_t *stack) {
    if (!stack) return;
    memset(stack, 0, sizeof(*stack));
}

void elx_stack_destroy(elx_stack_t *stack) {
    if (!stack) return;
    free(stack->items);
    stack->items = NULL;
    stack->count = 0U;
    stack->capacity = 0U;
}

bool elx_stack_push(elx_stack_t *stack, elx_value_t value) {
    if (!stack) return false;
    if (stack->count == stack->capacity) {
        size_t capacity = stack->capacity ? stack->capacity * 2U : ELX_VM_STACK_SIZE;
        elx_value_t *items = (elx_value_t *)realloc(stack->items, capacity * sizeof(*items));
        if (!items) return false;
        stack->items = items;
        stack->capacity = capacity;
    }
    stack->items[stack->count++] = value;
    return true;
}

elx_value_t elx_stack_pop(elx_stack_t *stack) {
    if (!stack || stack->count == 0U) return elx_value_nil();
    return stack->items[--stack->count];
}

elx_value_t elx_stack_peek(const elx_stack_t *stack, int32_t depth) {
    size_t index;
    if (!stack || stack->count == 0U) return elx_value_nil();
    if (depth >= 0) {
        index = stack->count - 1U - (size_t)depth;
    } else {
        index = 0U;
    }
    if (index >= stack->count) return elx_value_nil();
    return stack->items[index];
}
