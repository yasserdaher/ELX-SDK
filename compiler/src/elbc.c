#include "elx.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    unsigned char *data;
    size_t size;
    size_t capacity;
    bool failed;
} byte_buffer_t;

typedef struct {
    char *key;
    uint32_t index;
} string_entry_t;

typedef struct {
    char **strings;
    size_t count;
    size_t capacity;
} string_pool_t;

typedef struct {
    uint32_t next_register;
    string_pool_t string_pool;
    size_t code_base;
} elbc_context_t;

static void buffer_init(byte_buffer_t *buffer) {
    memset(buffer, 0, sizeof(*buffer));
}

static void buffer_reserve(byte_buffer_t *buffer, size_t additional) {
    size_t needed;
    size_t capacity;
    unsigned char *data;
    if (buffer->failed) return;
    needed = buffer->size + additional;
    if (needed <= buffer->capacity) return;
    capacity = buffer->capacity ? buffer->capacity : 256U;
    while (capacity < needed) capacity *= 2U;
    data = (unsigned char *)realloc(buffer->data, capacity);
    if (!data) {
        buffer->failed = true;
        return;
    }
    buffer->data = data;
    buffer->capacity = capacity;
}

static void buffer_write(byte_buffer_t *buffer, const void *bytes, size_t length) {
    buffer_reserve(buffer, length);
    if (buffer->failed) return;
    memcpy(buffer->data + buffer->size, bytes, length);
    buffer->size += length;
}

static void buffer_u8(byte_buffer_t *buffer, unsigned char value) {
    buffer_write(buffer, &value, 1U);
}

static void buffer_u16(byte_buffer_t *buffer, uint16_t value) {
    buffer_u8(buffer, (unsigned char)(value & 0xFFU));
    buffer_u8(buffer, (unsigned char)((value >> 8U) & 0xFFU));
}

static void buffer_u32(byte_buffer_t *buffer, uint32_t value) {
    buffer_u8(buffer, (unsigned char)(value & 0xFFU));
    buffer_u8(buffer, (unsigned char)((value >> 8U) & 0xFFU));
    buffer_u8(buffer, (unsigned char)((value >> 16U) & 0xFFU));
    buffer_u8(buffer, (unsigned char)((value >> 24U) & 0xFFU));
}

static void buffer_patch_u32(byte_buffer_t *buffer, size_t pos, uint32_t value) {
    if (pos + 4U <= buffer->size) {
        buffer->data[pos] = (unsigned char)(value & 0xFFU);
        buffer->data[pos + 1U] = (unsigned char)((value >> 8U) & 0xFFU);
        buffer->data[pos + 2U] = (unsigned char)((value >> 16U) & 0xFFU);
        buffer->data[pos + 3U] = (unsigned char)((value >> 24U) & 0xFFU);
    }
}

static void buffer_u64(byte_buffer_t *buffer, uint64_t value) {
    size_t i;
    for (i = 0U; i < 8U; ++i) buffer_u8(buffer, (unsigned char)((value >> (i * 8U)) & 0xFFU));
}

static void buffer_string(byte_buffer_t *buffer, const char *value) {
    size_t length = value ? strlen(value) : 0U;
    buffer_u32(buffer, (uint32_t)length);
    if (length > 0U) buffer_write(buffer, value, length);
}

static char *dup_string(const char *value) {
    size_t length;
    char *result;
    if (!value) return NULL;
    length = strlen(value);
    result = (char *)malloc(length + 1U);
    if (!result) return NULL;
    memcpy(result, value, length + 1U);
    return result;
}

static void buffer_string_pooled(byte_buffer_t *buffer, elbc_context_t *ctx, const char *value) {
    uint32_t index = 0U;
    size_t i;
    if (!value) {
        buffer_u32(buffer, 0U);
        return;
    }
    for (i = 0U; i < ctx->string_pool.count; ++i) {
        if (ctx->string_pool.strings[i] && strcmp(ctx->string_pool.strings[i], value) == 0) {
            index = (uint32_t)(i + 1U);
            break;
        }
    }
    buffer_u32(buffer, index);
}

static uint32_t string_pool_add(string_pool_t *pool, const char *value) {
    size_t i;
    char *copy;
    if (!value) return 0U;
    for (i = 0U; i < pool->count; ++i) {
        if (pool->strings[i] && strcmp(pool->strings[i], value) == 0) {
            return (uint32_t)(i + 1U);
        }
    }
    if (pool->count == pool->capacity) {
        size_t capacity = pool->capacity ? pool->capacity * 2U : 16U;
        char **strings = (char **)realloc(pool->strings, capacity * sizeof(*strings));
        if (!strings) return 0U;
        pool->strings = strings;
        pool->capacity = capacity;
    }
    copy = dup_string(value);
    if (!copy) return 0U;
    pool->strings[pool->count] = copy;
    ++pool->count;
    return (uint32_t)pool->count;
}

static void emit_string(byte_buffer_t *buffer, const char *value) {
    size_t length = value ? strlen(value) : 0U;
    buffer_u32(buffer, (uint32_t)length);
    if (length > 0U) buffer_write(buffer, value, length);
}





static void emit_load_const(byte_buffer_t *buffer, uint32_t index) {
    buffer_u8(buffer, 0x01U);
    buffer_u32(buffer, index);
}

static void emit_load_int(byte_buffer_t *buffer, uint64_t value) {
    buffer_u8(buffer, 0x02U);
    buffer_u64(buffer, value);
}

static void emit_load_float(byte_buffer_t *buffer, double value) {
    uint64_t bits;
    memcpy(&bits, &value, sizeof(bits));
    buffer_u8(buffer, 0x03U);
    buffer_u64(buffer, bits);
}

static void emit_load_bool(byte_buffer_t *buffer, bool value) {
    buffer_u8(buffer, 0x04U);
    buffer_u8(buffer, value ? 1U : 0U);
}

static void emit_load_nil(byte_buffer_t *buffer) {
    buffer_u8(buffer, 0x05U);
}

static void emit_binary(byte_buffer_t *buffer, elx_token_type_t operator_type) {
    unsigned char operation = 0U;
    switch (operator_type) {
        case ELX_TOKEN_PLUS: operation = 1U; break;
        case ELX_TOKEN_MINUS: operation = 2U; break;
        case ELX_TOKEN_STAR: operation = 3U; break;
        case ELX_TOKEN_SLASH: operation = 4U; break;
        case ELX_TOKEN_PERCENT: operation = 5U; break;
        case ELX_TOKEN_LT: operation = 6U; break;
        case ELX_TOKEN_LT_EQ: operation = 7U; break;
        case ELX_TOKEN_GT: operation = 8U; break;
        case ELX_TOKEN_GT_EQ: operation = 9U; break;
        case ELX_TOKEN_EQ:
        case ELX_TOKEN_EQ_EQ: operation = 10U; break;
        case ELX_TOKEN_BANG_EQ: operation = 11U; break;
        case ELX_TOKEN_AND:
        case ELX_TOKEN_AND_AND: operation = 12U; break;
        case ELX_TOKEN_OR:
        case ELX_TOKEN_OR_OR: operation = 13U; break;
        default: operation = 0U; break;
    }
    buffer_u8(buffer, 0x0BU);
    buffer_u8(buffer, operation);
}

static void emit_unary(byte_buffer_t *buffer, elx_token_type_t operator_type) {
    unsigned char operation = operator_type == ELX_TOKEN_BANG ? 1U : 2U;
    buffer_u8(buffer, 0x0CU);
    buffer_u8(buffer, operation);
}

static void emit_return(byte_buffer_t *buffer) {
    buffer_u8(buffer, 0x0AU);
}

static void emit_return_tail(byte_buffer_t *buffer) {
    buffer_u8(buffer, 0x0AU);
    buffer_u8(buffer, 0x01U);
}

static void emit_call(byte_buffer_t *buffer, const elx_ast_t *node, elbc_context_t *ctx) {
    buffer_u8(buffer, 0x09U);
    buffer_string_pooled(buffer, ctx, node->func_name ? node->func_name : "");
    buffer_u16(buffer, (uint16_t)elx_ast_count(node->args));
}

static void emit_call_tail(byte_buffer_t *buffer, const elx_ast_t *node, elbc_context_t *ctx) {
    buffer_u8(buffer, 0x20U);
    buffer_string_pooled(buffer, ctx, node->func_name ? node->func_name : "");
    buffer_u16(buffer, (uint16_t)elx_ast_count(node->args));
}

static void emit_widget_create(byte_buffer_t *buffer, const char *name, elbc_context_t *ctx) {
    buffer_u8(buffer, 0x14U);
    buffer_string_pooled(buffer, ctx, name);
}

static void emit_widget_property(byte_buffer_t *buffer, const char *name, elbc_context_t *ctx) {
    buffer_u8(buffer, 0x15U);
    buffer_string_pooled(buffer, ctx, name);
}

static void emit_state_subscribe(byte_buffer_t *buffer, const char *name, elbc_context_t *ctx) {
    buffer_u8(buffer, 0x16U);
    buffer_string_pooled(buffer, ctx, name);
}

static bool is_widget_name(const char *name) {
    static const char *widgets[] = {
        "Window", "Button", "Text", "Input", "Panel", "List", "Icon",
        "Image", "Column", "Row", "Grid", "Stack", "Scroll", "Slider",
        "Checkbox", "Radio", "Tab", "Menu", "Tooltip", "Dialog",
        "Toast", "Card", "Badge", "Avatar", "Divider", "Spinner",
        "Switch", "Control", "Launcher", "Lock", "Desktop",
        "Separator", "Menubar", "Statusbar", "Toolbar", "Dropdown",
        "Label", "Container", "Box", "Frame", "SVG", NULL
    };
    size_t i;
    if (!name) return false;
    for (i = 0; widgets[i]; ++i) {
        if (strcmp(widgets[i], name) == 0) return true;
    }
    return false;
}

static void emit_widget_inline(byte_buffer_t *buffer, const elx_ast_t *widget, elbc_context_t *ctx);

static void emit_expression(byte_buffer_t *buffer, const elx_ast_t *node, elbc_context_t *ctx);

static void emit_statement(byte_buffer_t *buffer, const elx_ast_t *node, elbc_context_t *ctx);

static void emit_widget_inline(byte_buffer_t *buffer, const elx_ast_t *widget, elbc_context_t *ctx) {
    const elx_ast_t *property;
    const elx_ast_t *child;
    emit_widget_create(buffer, widget->name, ctx);
    buffer_u8(buffer, 0x0FU);
    for (property = widget->properties; property; property = property->next) {
        if (property->kind == ELX_AST_UI_PROPERTY || property->kind == ELX_AST_UI_EVENT) {
            emit_expression(buffer, property->value, ctx);
            emit_widget_property(buffer, property->name, ctx);
        }
    }
    buffer_u8(buffer, 0x10U);
    for (child = widget->children; child; child = child->next) {
        if (child->kind == ELX_AST_STRUCT_LITERAL && is_widget_name(child->name)) {
            emit_widget_inline(buffer, child, ctx);
        }
    }
}

static void emit_store_local(byte_buffer_t *buffer, uint32_t index) {
    buffer_u8(buffer, 0x06U);
    buffer_u32(buffer, index);
}

static void emit_load_local(byte_buffer_t *buffer, uint32_t index) {
    buffer_u8(buffer, 0x07U);
    buffer_u32(buffer, index);
}



static void emit_loop_start(byte_buffer_t *buffer) {
    buffer_u8(buffer, 0x1DU);
}

static void emit_loop_end(byte_buffer_t *buffer, uint32_t target_offset) {
    buffer_u8(buffer, 0x1EU);
    buffer_u32(buffer, target_offset);
}

static size_t count_functions(const elx_ast_t *node) {
    size_t count = 0U;
    for (; node; node = node->next) {
        if (node->kind == ELX_AST_FUNCTION) ++count;
        count += count_functions(node->body);
        count += count_functions(node->condition);
        count += count_functions(node->value);
        count += count_functions(node->left);
        count += count_functions(node->right);
        count += count_functions(node->children);
        count += count_functions(node->items);
        count += count_functions(node->properties);
    }
    return count;
}

static size_t count_imports(const elx_ast_t *node) {
    size_t count = 0U;
    for (; node; node = node->next) {
        if (node->kind == ELX_AST_IMPORT) ++count;
    }
    return count;
}

static size_t count_exports(const elx_ast_t *node) {
    size_t count = 0U;
    for (; node; node = node->next) {
        if (node->kind == ELX_AST_EXPORT && node->value &&
            (node->value->kind == ELX_AST_FUNCTION ||
             node->value->kind == ELX_AST_CONST ||
             node->value->kind == ELX_AST_VAR)) {
            ++count;
        }
    }
    return count;
}

static size_t count_widgets(const elx_ast_t *node) {
    size_t count = 0U;
    for (; node; node = node->next) {
        if (node->kind == ELX_AST_WIDGET) ++count;
        count += count_widgets(node->children);
        count += count_widgets(node->body);
        count += count_widgets(node->condition);
        count += count_widgets(node->value);
        count += count_widgets(node->left);
        count += count_widgets(node->right);
        count += count_widgets(node->items);
        count += count_widgets(node->properties);
    }
    return count;
}

static void emit_expression(byte_buffer_t *buffer, const elx_ast_t *node, elbc_context_t *ctx) {
    const elx_ast_t *child;
    uint32_t reg;
    if (!node) return;
    switch (node->kind) {
        case ELX_AST_INT:
            emit_load_int(buffer, node->integer);
            break;
        case ELX_AST_FLOAT:
            emit_load_float(buffer, node->real);
            break;
        case ELX_AST_STRING: {
            uint32_t str_index = string_pool_add(&ctx->string_pool, node->text ? node->text : "");
            emit_load_const(buffer, str_index);
            break;
        }
        case ELX_AST_BOOL:
            emit_load_bool(buffer, node->boolean);
            break;
        case ELX_AST_NIL:
            emit_load_nil(buffer);
            break;
        case ELX_AST_IDENT:
            reg = node->vm_slot;
            emit_load_local(buffer, reg);
            break;
        case ELX_AST_BINARY:
            emit_expression(buffer, node->left, ctx);
            emit_expression(buffer, node->right, ctx);
            emit_binary(buffer, node->operator);
            break;
        case ELX_AST_UNARY:
            emit_expression(buffer, node->left, ctx);
            emit_unary(buffer, node->operator);
            break;
        case ELX_AST_CALL:
            if (node->func_name) {
                for (child = node->args; child; child = child->next) emit_expression(buffer, child, ctx);
                if (node->tail_call && ctx->next_register > 0U) {
                    emit_call_tail(buffer, node, ctx);
                } else {
                    emit_call(buffer, node, ctx);
                }
            } else {
                emit_expression(buffer, node->callee, ctx);
                for (child = node->args; child; child = child->next) emit_expression(buffer, child, ctx);
                emit_call(buffer, node, ctx);
            }
            break;
        case ELX_AST_MEMBER:
            emit_expression(buffer, node->object, ctx);
            buffer_u8(buffer, 0x0DU);
            buffer_string_pooled(buffer, ctx, node->name);
            break;
        case ELX_AST_INDEX:
            emit_expression(buffer, node->object, ctx);
            emit_expression(buffer, node->value, ctx);
            buffer_u8(buffer, 0x0EU);
            break;
        case ELX_AST_STRUCT_LITERAL:
            if (node->stack_allocated) {
                buffer_u8(buffer, 0x0FU);
            }
            if (is_widget_name(node->name)) {
                emit_widget_inline(buffer, node, ctx);
                emit_load_nil(buffer);
            } else {
                emit_load_const(buffer, 0U);
            }
            break;
        case ELX_AST_ARRAY:
            emit_load_const(buffer, 0U);
            break;
        case ELX_AST_LAMBDA:
            emit_load_const(buffer, 0U);
            break;
        case ELX_AST_UI_PROPERTY:
            emit_expression(buffer, node->value, ctx);
            break;
        case ELX_AST_UI_EVENT:
            emit_expression(buffer, node->value, ctx);
            break;
        case ELX_AST_ASSIGN:
            emit_expression(buffer, node->right, ctx);
            if (node->left && node->left->kind == ELX_AST_IDENT) {
                emit_store_local(buffer, node->left->vm_slot);
            }
            break;
        default:
            emit_load_nil(buffer);
            break;
    }
}

static void emit_statement(byte_buffer_t *buffer, const elx_ast_t *node, elbc_context_t *ctx) {
    const elx_ast_t *child;
    if (!node) return;
    switch (node->kind) {
        case ELX_AST_BLOCK:
            buffer_u8(buffer, 0x0FU);
            for (child = node->next; child; child = child->next) emit_statement(buffer, child, ctx);
            buffer_u8(buffer, 0x10U);
            break;
        case ELX_AST_VAR:
        case ELX_AST_CONST:
            emit_expression(buffer, node->value, ctx);
            emit_store_local(buffer, node->vm_slot);
            break;
        case ELX_AST_EXPR_STMT:
            emit_expression(buffer, node->value, ctx);
            break;
        case ELX_AST_RETURN:
            if (node->value && node->value->kind == ELX_AST_CALL && node->value->tail_call) {
                emit_expression(buffer, node->value, ctx);
                emit_return_tail(buffer);
            } else {
                emit_expression(buffer, node->value, ctx);
                emit_return(buffer);
            }
            break;
        case ELX_AST_IF:
            emit_expression(buffer, node->condition, ctx);
            buffer_u8(buffer, 0x1BU);
            size_t jif_pos = buffer->size;
            buffer_u32(buffer, 0U);
            emit_statement(buffer, node->body, ctx);
            if (node->else_body) {
                buffer_u8(buffer, 0x1AU);
                size_t jmp_pos = buffer->size;
                buffer_u32(buffer, 0U);
                buffer_patch_u32(buffer, jif_pos, (uint32_t)(buffer->size - ctx->code_base));
                emit_statement(buffer, node->else_body, ctx);
                buffer_patch_u32(buffer, jmp_pos, (uint32_t)(buffer->size - ctx->code_base));
            } else {
                buffer_patch_u32(buffer, jif_pos, (uint32_t)(buffer->size - ctx->code_base));
            }
            break;
        case ELX_AST_WHILE:
            buffer_u8(buffer, 0x1DU);
            uint32_t loop_start_pc = (uint32_t)(buffer->size - ctx->code_base);
            emit_expression(buffer, node->condition, ctx);
            buffer_u8(buffer, 0x1BU);
            size_t wif_pos = buffer->size;
            buffer_u32(buffer, 0U);
            emit_statement(buffer, node->body, ctx);
            buffer_u8(buffer, 0x1A);
            buffer_u32(buffer, loop_start_pc);
            buffer_patch_u32(buffer, wif_pos, (uint32_t)(buffer->size - ctx->code_base));
            break;
        case ELX_AST_FOR:
            buffer_u8(buffer, 0x1DU);
            uint32_t for_start_pc = (uint32_t)(buffer->size - ctx->code_base);
            emit_expression(buffer, node->condition, ctx);
            buffer_u8(buffer, 0x1BU);
            size_t fif_pos = buffer->size;
            buffer_u32(buffer, 0U);
            emit_statement(buffer, node->body, ctx);
            buffer_u8(buffer, 0x1A);
            buffer_u32(buffer, for_start_pc);
            buffer_patch_u32(buffer, fif_pos, (uint32_t)(buffer->size - ctx->code_base));
            break;
        case ELX_AST_LOOP:
            emit_loop_start(buffer);
            emit_statement(buffer, node->body, ctx);
            emit_loop_end(buffer, 0U);
            break;
        case ELX_AST_BREAK:
            buffer_u8(buffer, 0x1EU);
            buffer_u32(buffer, 0U);
            break;
        case ELX_AST_CONTINUE:
            buffer_u8(buffer, 0x1DU);
            break;
        default:
            emit_expression(buffer, node, ctx);
            break;
    }
}

static void assign_vm_slots(elx_ast_t *node, uint32_t *next_slot) {
    if (!node) return;
    if (node->kind == ELX_AST_IDENT && node->name) {
        node->vm_slot = *next_slot;
        ++*next_slot;
    }
    if (node->kind == ELX_AST_VAR || node->kind == ELX_AST_CONST) {
        node->vm_slot = *next_slot;
        ++*next_slot;
    }
    assign_vm_slots(node->left, next_slot);
    assign_vm_slots(node->right, next_slot);
    assign_vm_slots(node->third, next_slot);
    assign_vm_slots(node->body, next_slot);
    assign_vm_slots(node->else_body, next_slot);
    assign_vm_slots(node->condition, next_slot);
    assign_vm_slots(node->init, next_slot);
    assign_vm_slots(node->step, next_slot);
    assign_vm_slots(node->target, next_slot);
    assign_vm_slots(node->value, next_slot);
    assign_vm_slots(node->callee, next_slot);
    for (elx_ast_t *child = node->args; child; child = child->next)
        assign_vm_slots(child, next_slot);
    assign_vm_slots(node->object, next_slot);
    for (elx_ast_t *child = node->items; child; child = child->next)
        assign_vm_slots(child, next_slot);
    for (elx_ast_t *child = node->properties; child; child = child->next)
        assign_vm_slots(child, next_slot);
    for (elx_ast_t *child = node->children; child; child = child->next)
        assign_vm_slots(child, next_slot);
    for (elx_ast_t *child = node->next; child; child = child->next)
        assign_vm_slots(child, next_slot);
}

static void remap_params_in_tree(elx_ast_t *node, const elx_ast_t *params, uint32_t start_slot, int depth) {
    if (!node) return;
    if (node->kind == ELX_AST_IDENT && node->name) {
        uint32_t i = 0U;
        for (elx_ast_t *p = (elx_ast_t *)params; p; p = p->next, ++i) {
            if (p->name && node->name && strcmp(p->name, node->name) == 0) {
                node->vm_slot = start_slot + i;
                break;
            }
        }
    }
    remap_params_in_tree(node->left, params, start_slot, depth+1);
    remap_params_in_tree(node->right, params, start_slot, depth+1);
    remap_params_in_tree(node->third, params, start_slot, depth+1);
    remap_params_in_tree(node->body, params, start_slot, depth+1);
    remap_params_in_tree(node->else_body, params, start_slot, depth+1);
    remap_params_in_tree(node->condition, params, start_slot, depth+1);
    remap_params_in_tree(node->init, params, start_slot, depth+1);
    remap_params_in_tree(node->step, params, start_slot, depth+1);
    remap_params_in_tree(node->target, params, start_slot, depth+1);
    remap_params_in_tree(node->value, params, start_slot, depth+1);
    remap_params_in_tree(node->callee, params, start_slot, depth+1);
    for (elx_ast_t *c = node->args; c; c = c->next)
        remap_params_in_tree(c, params, start_slot, depth+1);
    remap_params_in_tree(node->object, params, start_slot, depth+1);
    for (elx_ast_t *c = node->items; c; c = c->next)
        remap_params_in_tree(c, params, start_slot, depth+1);
    for (elx_ast_t *c = node->properties; c; c = c->next)
        remap_params_in_tree(c, params, start_slot, depth+1);
    for (elx_ast_t *c = node->children; c; c = c->next)
        remap_params_in_tree(c, params, start_slot, depth+1);
    for (elx_ast_t *c = node->next; c; c = c->next)
        remap_params_in_tree(c, params, start_slot, depth+1);
}

typedef struct {
    const char *name;
    uint32_t slot;
} var_map_t;

static void remap_locals_in_tree(elx_ast_t *node, var_map_t *vars, uint32_t var_count, int depth) {
    if (!node) return;
    if (node->kind == ELX_AST_VAR || node->kind == ELX_AST_CONST) {
        if (node->name) {
            for (uint32_t i = 0U; i < var_count; ++i) {
                if (vars[i].name && node->name && strcmp(vars[i].name, node->name) == 0) {
                    node->vm_slot = vars[i].slot;
                    break;
                }
            }
        }
    }
    if (node->kind == ELX_AST_IDENT && node->name) {
        for (uint32_t i = 0U; i < var_count; ++i) {
            if (vars[i].name && node->name && strcmp(vars[i].name, node->name) == 0) {
                node->vm_slot = vars[i].slot;
                break;
            }
        }
    }
    remap_locals_in_tree(node->left, vars, var_count, depth+1);
    remap_locals_in_tree(node->right, vars, var_count, depth+1);
    remap_locals_in_tree(node->third, vars, var_count, depth+1);
    remap_locals_in_tree(node->body, vars, var_count, depth+1);
    remap_locals_in_tree(node->else_body, vars, var_count, depth+1);
    remap_locals_in_tree(node->condition, vars, var_count, depth+1);
    remap_locals_in_tree(node->init, vars, var_count, depth+1);
    remap_locals_in_tree(node->step, vars, var_count, depth+1);
    remap_locals_in_tree(node->target, vars, var_count, depth+1);
    remap_locals_in_tree(node->value, vars, var_count, depth+1);
    remap_locals_in_tree(node->callee, vars, var_count, depth+1);
    for (elx_ast_t *c = node->args; c; c = c->next)
        remap_locals_in_tree(c, vars, var_count, depth+1);
    remap_locals_in_tree(node->object, vars, var_count, depth+1);
    for (elx_ast_t *c = node->items; c; c = c->next)
        remap_locals_in_tree(c, vars, var_count, depth+1);
    for (elx_ast_t *c = node->properties; c; c = c->next)
        remap_locals_in_tree(c, vars, var_count, depth+1);
    for (elx_ast_t *c = node->children; c; c = c->next)
        remap_locals_in_tree(c, vars, var_count, depth+1);
    for (elx_ast_t *c = node->next; c; c = c->next)
        remap_locals_in_tree(c, vars, var_count, depth+1);
}

static void collect_locals(elx_ast_t *node, var_map_t *vars, uint32_t *count, uint32_t *next_slot) {
    if (!node) return;
    if (node->kind == ELX_AST_VAR || node->kind == ELX_AST_CONST) {
        if (*count < 256 && node->name) {
            vars[*count].name = node->name;
            vars[*count].slot = *next_slot;
            ++*count;
            ++*next_slot;
        }
    }
    collect_locals(node->left, vars, count, next_slot);
    collect_locals(node->right, vars, count, next_slot);
    collect_locals(node->third, vars, count, next_slot);
    collect_locals(node->body, vars, count, next_slot);
    collect_locals(node->else_body, vars, count, next_slot);
    collect_locals(node->condition, vars, count, next_slot);
    collect_locals(node->init, vars, count, next_slot);
    collect_locals(node->step, vars, count, next_slot);
    collect_locals(node->target, vars, count, next_slot);
    collect_locals(node->value, vars, count, next_slot);
    collect_locals(node->callee, vars, count, next_slot);
    for (elx_ast_t *c = node->args; c; c = c->next)
        collect_locals(c, vars, count, next_slot);
    collect_locals(node->object, vars, count, next_slot);
    for (elx_ast_t *c = node->items; c; c = c->next)
        collect_locals(c, vars, count, next_slot);
    for (elx_ast_t *c = node->properties; c; c = c->next)
        collect_locals(c, vars, count, next_slot);
    for (elx_ast_t *c = node->children; c; c = c->next)
        collect_locals(c, vars, count, next_slot);
    for (elx_ast_t *c = node->next; c; c = c->next)
        collect_locals(c, vars, count, next_slot);
}

static void emit_function(byte_buffer_t *buffer, elx_ast_t *function, elbc_context_t *ctx) {
    uint32_t argument_count = (uint32_t)elx_ast_count(function->args);
    uint32_t start_slot = ctx->next_register;
    uint32_t local_count = 0U;
    uint32_t next_local = start_slot + argument_count;
    var_map_t local_vars[256];
    size_t code_start;
    uint32_t code_size = 0U;
    remap_params_in_tree(function->body, function->args, start_slot, 0);
    collect_locals(function->body, local_vars, &local_count, &next_local);
    remap_locals_in_tree(function->body, local_vars, local_count, 0);
    if (ctx->next_register < next_local) {
        ctx->next_register = next_local;
    }
    emit_string(buffer, function->name);
    buffer_u32(buffer, argument_count);
    buffer_u32(buffer, start_slot);
    buffer_u8(buffer, function->public_value ? 1U : 0U);
    code_start = buffer->size;
    buffer_u32(buffer, 0U);
    ctx->code_base = code_start + 4U;
    emit_statement(buffer, function->body, ctx);
    if (function->tail_call) {
        emit_load_nil(buffer);
    }
    emit_return(buffer);
    code_size = (uint32_t)(buffer->size - code_start - 4U);
    buffer->data[code_start] = (unsigned char)(code_size & 0xFFU);
    buffer->data[code_start + 1U] = (unsigned char)((code_size >> 8U) & 0xFFU);
    buffer->data[code_start + 2U] = (unsigned char)((code_size >> 16U) & 0xFFU);
    buffer->data[code_start + 3U] = (unsigned char)((code_size >> 24U) & 0xFFU);
}

static void emit_widget(byte_buffer_t *buffer, const elx_ast_t *widget, elbc_context_t *ctx) {
    const elx_ast_t *property;
    const elx_ast_t *child;
    emit_widget_create(buffer, widget->name, ctx);
    for (property = widget->properties; property; property = property->next) {
        if (property->kind == ELX_AST_UI_PROPERTY || property->kind == ELX_AST_UI_EVENT) {
            emit_expression(buffer, property->value, ctx);
            emit_widget_property(buffer, property->name, ctx);
        }
    }
    for (child = widget->children; child; child = child->next) {
        emit_widget(buffer, child, ctx);
    }
}

static void write_string_table(byte_buffer_t *buffer, elbc_context_t *ctx) {
    size_t i;
    buffer_u32(buffer, (uint32_t)ctx->string_pool.count);
    for (i = 0U; i < ctx->string_pool.count; ++i) {
        buffer_string(buffer, ctx->string_pool.strings[i]);
    }
}

static void write_import_table(byte_buffer_t *buffer, const elx_ast_t *program, elbc_context_t *ctx) {
    const elx_ast_t *node;
    (void)ctx;
    for (node = program; node; node = node->next) {
        if (node->kind == ELX_AST_IMPORT) {
            emit_string(buffer, node->name);
            emit_string(buffer, node->import_path);
            emit_string(buffer, node->alias);
        }
    }
}

static void write_export_table(byte_buffer_t *buffer, const elx_ast_t *program, elbc_context_t *ctx) {
    const elx_ast_t *node;
    const elx_ast_t *fn;
    uint32_t func_index;
    (void)ctx;
    for (node = program; node; node = node->next) {
        if (node->kind == ELX_AST_EXPORT && node->value &&
            (node->value->kind == ELX_AST_FUNCTION)) {
            fn = node->value;
            func_index = 0U;
            for (const elx_ast_t *scan = program; scan; scan = scan->next) {
                if (scan == node) break;
                if (scan->kind == ELX_AST_FUNCTION) ++func_index;
                else if (scan->kind == ELX_AST_EXPORT && scan->value &&
                         scan->value->kind == ELX_AST_FUNCTION) ++func_index;
            }
            emit_string(buffer, fn->name);
            buffer_u32(buffer, func_index);
        }
    }
}

static void emit_program(byte_buffer_t *buffer, elx_ast_t *program, elbc_context_t *ctx) {
    elx_ast_t *declaration;
    for (declaration = program; declaration; declaration = declaration->next) {
        if (declaration->kind == ELX_AST_FUNCTION) {
            emit_function(buffer, declaration, ctx);
        } else if (declaration->kind == ELX_AST_EXPORT && declaration->value &&
                   declaration->value->kind == ELX_AST_FUNCTION) {
            emit_function(buffer, declaration->value, ctx);
        }
        /* Widget and state declarations are counted in the header (widget_count)
         * but their opcode streams are not emitted into the module body. The
         * runtime reader does not consume module-level widget/state instruction
         * bytes; emitting them previously caused reader/writer misalignment
         * (ELBC 3.2). Widget reconstruction requires a framed section in a
         * future format revision. */
    }
    write_import_table(buffer, program, ctx);
    write_string_table(buffer, ctx);
    write_export_table(buffer, program, ctx);
}

static uint32_t crc32(const unsigned char *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFU;
    size_t i;
    int bit;
    for (i = 0U; i < length; ++i) {
        crc ^= data[i];
        for (bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1U) ^ (0xEDB88320U & (uint32_t)-((int32_t)(crc & 1U)));
        }
    }
    return ~crc;
}

static void string_pool_init(string_pool_t *pool) {
    memset(pool, 0, sizeof(*pool));
}

static void string_pool_destroy(string_pool_t *pool) {
    size_t i;
    for (i = 0U; i < pool->count; ++i) {
        free(pool->strings[i]);
    }
    free(pool->strings);
    memset(pool, 0, sizeof(*pool));
}

static uint32_t collect_strings(const elx_ast_t *node, elbc_context_t *ctx) {
    const elx_ast_t *child;
    uint32_t base_index;
    if (!node) return 0;
    base_index = (uint32_t)(ctx->string_pool.count + 1U);
    if (node->kind == ELX_AST_STRING && node->text) {
        string_pool_add(&ctx->string_pool, node->text);
    }
    if (node->kind == ELX_AST_CALL && node->func_name) {
        string_pool_add(&ctx->string_pool, node->func_name);
    }
    if (node->name && (node->kind == ELX_AST_IDENT || node->kind == ELX_AST_FUNCTION ||
                       node->kind == ELX_AST_WIDGET || node->kind == ELX_AST_STATE ||
                       node->kind == ELX_AST_STRUCT_LITERAL ||
                       node->kind == ELX_AST_UI_PROPERTY ||
                       node->kind == ELX_AST_UI_EVENT ||
                       node->kind == ELX_AST_IMPORT)) {
        string_pool_add(&ctx->string_pool, node->name);
    }
    if (node->kind == ELX_AST_IMPORT) {
        if (node->module_name) string_pool_add(&ctx->string_pool, node->module_name);
        if (node->alias) string_pool_add(&ctx->string_pool, node->alias);
    }
    for (child = node->left; child; child = child->next) collect_strings(child, ctx);
    for (child = node->right; child; child = child->next) collect_strings(child, ctx);
    for (child = node->third; child; child = child->next) collect_strings(child, ctx);
    for (child = node->body; child; child = child->next) collect_strings(child, ctx);
    for (child = node->else_body; child; child = child->next) collect_strings(child, ctx);
    for (child = node->condition; child; child = child->next) collect_strings(child, ctx);
    for (child = node->init; child; child = child->next) collect_strings(child, ctx);
    for (child = node->step; child; child = child->next) collect_strings(child, ctx);
    for (child = node->target; child; child = child->next) collect_strings(child, ctx);
    for (child = node->value; child; child = child->next) collect_strings(child, ctx);
    for (child = node->callee; child; child = child->next) collect_strings(child, ctx);
    for (child = node->args; child; child = child->next) collect_strings(child, ctx);
    collect_strings(node->object, ctx);
    for (child = node->items; child; child = child->next) collect_strings(child, ctx);
    for (child = node->properties; child; child = child->next) collect_strings(child, ctx);
    for (child = node->children; child; child = child->next) collect_strings(child, ctx);
    for (child = node->next; child; child = child->next) collect_strings(child, ctx);
    return base_index;
}

bool elx_write_elbc(const elx_ast_t *program, const char *output_path, elx_diagnostic_list_t *diagnostics) {
    byte_buffer_t buffer;
    elbc_context_t ctx;
    FILE *file;
    size_t function_count;
    size_t widget_count;
    size_t string_count;
    size_t import_count;
    size_t export_count;
    uint32_t checksum;
    uint32_t next_slot = 0U;
    if (!program || !output_path || !diagnostics) return false;
    memset(&ctx, 0, sizeof(ctx));
    string_pool_init(&ctx.string_pool);
    buffer_init(&buffer);
    collect_strings(program, &ctx);
    (void)count_widgets; /* AST widget count not serialized in ELBC 3.x module body */
    function_count = count_functions(program);
    widget_count = 0U; /* serialized widget records in body (none in 3.x; header reflects actual content) */
    string_count = ctx.string_pool.count;
    import_count = count_imports(program);
    export_count = count_exports(program);
    buffer_u8(&buffer, 'E');
    buffer_u8(&buffer, 'L');
    buffer_u8(&buffer, 'X');
    buffer_u8(&buffer, 'B');
    buffer_u8(&buffer, 0x03U);
    buffer_u8(&buffer, 0x02U);
    buffer_u16(&buffer, 0U); /* flags */
    buffer_u64(&buffer, 0U); /* source_size_or_timestamp (u64 @8): writer uses source_size; reader aliases timestamp — same layout, preserved for 3.1/3.2 */
    buffer_u32(&buffer, 0U); /* reserved @16 */
    buffer_u32(&buffer, (uint32_t)function_count);
    buffer_u32(&buffer, (uint32_t)widget_count);
    buffer_u32(&buffer, (uint32_t)string_count);
    buffer_u32(&buffer, next_slot);
    buffer_u32(&buffer, (uint32_t)import_count);
    buffer_u32(&buffer, (uint32_t)export_count);
    assign_vm_slots((elx_ast_t *)program, &next_slot);
    emit_program(&buffer, (elx_ast_t *)program, &ctx);
    checksum = crc32(buffer.data, buffer.size);
    buffer_u32(&buffer, checksum);
    file = fopen(output_path, "wb");
    if (!file) {
        elx_diagnostic_add(diagnostics, 400, "cannot open output file", (elx_location_t){1U, 1U, 0U});
        free(buffer.data);
        string_pool_destroy(&ctx.string_pool);
        return false;
    }
    if (buffer.data && buffer.size > 0U && fwrite(buffer.data, 1U, buffer.size, file) != buffer.size) {
        elx_diagnostic_add(diagnostics, 401, "cannot write output file", (elx_location_t){1U, 1U, 0U});
        fclose(file);
        free(buffer.data);
        string_pool_destroy(&ctx.string_pool);
        return false;
    }
    fclose(file);
    free(buffer.data);
    string_pool_destroy(&ctx.string_pool);
    return true;
}
