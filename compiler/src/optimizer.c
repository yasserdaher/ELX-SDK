#include "elx.h"
#include <stdlib.h>
#include <string.h>

static char *copy_string(const char *value) {
    size_t length;
    char *result;
    if (!value) return NULL;
    length = strlen(value);
    result = (char *)malloc(length + 1U);
    if (!result) return NULL;
    memcpy(result, value, length + 1U);
    return result;
}

static void append_update_order(elx_optimization_report_t *report, const char *name) {
    char **items;
    size_t count = report->update_order_count;
    items = (char **)realloc(report->update_order, (count + 1U) * sizeof(*items));
    if (!items) return;
    report->update_order = items;
    report->update_order[count] = copy_string(name);
    if (report->update_order[count]) ++report->update_order_count;
}

static void count_nodes(const elx_ast_t *node, elx_optimization_report_t *report) {
    const elx_ast_t *child;
    if (!node) return;
    switch (node->kind) {
        case ELX_AST_FUNCTION:
            ++report->function_count;
            break;
        case ELX_AST_STATE:
            ++report->state_count;
            break;
        case ELX_AST_WIDGET:
            ++report->widget_count;
            break;
        case ELX_AST_PERMISSIONS:
        case ELX_AST_CAPABILITY:
            ++report->capability_count;
            break;
        default:
            break;
    }
    for (child = node->next; child; child = child->next) count_nodes(child, report);
    for (child = node->body; child; child = child->next) count_nodes(child, report);
    for (child = node->else_body; child; child = child->next) count_nodes(child, report);
    for (child = node->condition; child; child = child->next) count_nodes(child, report);
    for (child = node->init; child; child = child->next) count_nodes(child, report);
    for (child = node->step; child; child = child->next) count_nodes(child, report);
    for (child = node->target; child; child = child->next) count_nodes(child, report);
    for (child = node->value; child; child = child->next) count_nodes(child, report);
    for (child = node->left; child; child = child->next) count_nodes(child, report);
    for (child = node->right; child; child = child->next) count_nodes(child, report);
    for (child = node->third; child; child = child->next) count_nodes(child, report);
    for (child = node->callee; child; child = child->next) count_nodes(child, report);
    for (child = node->args; child; child = child->next) count_nodes(child, report);
    for (child = node->object; child; child = child->next) count_nodes(child, report);
    for (child = node->items; child; child = child->next) count_nodes(child, report);
    for (child = node->properties; child; child = child->next) count_nodes(child, report);
    for (child = node->children; child; child = child->next) count_nodes(child, report);
}

static bool is_constant(const elx_ast_t *node) {
    if (!node) return false;
    switch (node->kind) {
        case ELX_AST_INT:
        case ELX_AST_FLOAT:
        case ELX_AST_STRING:
        case ELX_AST_BOOL:
        case ELX_AST_NIL:
            return true;
        case ELX_AST_BINARY:
            return is_constant(node->left) && is_constant(node->right);
        case ELX_AST_UNARY:
            return is_constant(node->left);
        default:
            return false;
    }
}

static void release_ast_contents(elx_ast_t *node) {
    if (!node) return;
    elx_type_destroy(node->type);
    node->type = NULL;
    free(node->name);
    node->name = NULL;
    free(node->text);
    node->text = NULL;
    free(node->layout_hint);
    node->layout_hint = NULL;
    elx_ast_destroy(node->next);
    node->next = NULL;
    elx_ast_destroy(node->left);
    node->left = NULL;
    elx_ast_destroy(node->right);
    node->right = NULL;
    elx_ast_destroy(node->third);
    node->third = NULL;
    elx_ast_destroy(node->body);
    node->body = NULL;
    elx_ast_destroy(node->else_body);
    node->else_body = NULL;
    elx_ast_destroy(node->condition);
    node->condition = NULL;
    elx_ast_destroy(node->init);
    node->init = NULL;
    elx_ast_destroy(node->step);
    node->step = NULL;
    elx_ast_destroy(node->target);
    node->target = NULL;
    elx_ast_destroy(node->value);
    node->value = NULL;
    elx_ast_destroy(node->callee);
    node->callee = NULL;
    elx_ast_destroy(node->args);
    node->args = NULL;
    elx_ast_destroy(node->object);
    node->object = NULL;
    elx_ast_destroy(node->items);
    node->items = NULL;
    elx_ast_destroy(node->properties);
    node->properties = NULL;
    elx_ast_destroy(node->children);
    node->children = NULL;
}

static void steal_ast_contents(elx_ast_t *destination, elx_ast_t *source) {
    release_ast_contents(destination);
    *destination = *source;
    memset(source, 0, sizeof(*source));
    elx_ast_destroy(source);
}

static void replace_with_child(elx_ast_t *node, elx_ast_t *child, bool left_side) {
    if (!child) return;
    if (left_side) {
        node->left = NULL;
    } else {
        node->right = NULL;
    }
    steal_ast_contents(node, child);
}

static bool is_zero(const elx_ast_t *node) {
    return node && ((node->kind == ELX_AST_INT && node->integer == 0U) ||
                    (node->kind == ELX_AST_FLOAT && node->real == 0.0));
}

static bool is_one(const elx_ast_t *node) {
    return node && ((node->kind == ELX_AST_INT && node->integer == 1U) ||
                    (node->kind == ELX_AST_FLOAT && node->real == 1.0));
}

static void simplify_expressions(elx_ast_t *node, elx_optimization_report_t *report) {
    elx_ast_t *child;
    if (!node) return;
    for (child = node->left; child; child = child->next) simplify_expressions(child, report);
    for (child = node->right; child; child = child->next) simplify_expressions(child, report);
    for (child = node->third; child; child = child->next) simplify_expressions(child, report);
    for (child = node->body; child; child = child->next) simplify_expressions(child, report);
    for (child = node->else_body; child; child = child->next) simplify_expressions(child, report);
    for (child = node->condition; child; child = child->next) simplify_expressions(child, report);
    for (child = node->init; child; child = child->next) simplify_expressions(child, report);
    for (child = node->step; child; child = child->next) simplify_expressions(child, report);
    for (child = node->target; child; child = child->next) simplify_expressions(child, report);
    for (child = node->value; child; child = child->next) simplify_expressions(child, report);
    for (child = node->callee; child; child = child->next) simplify_expressions(child, report);
    for (child = node->args; child; child = child->next) simplify_expressions(child, report);
    for (child = node->object; child; child = child->next) simplify_expressions(child, report);
    for (child = node->items; child; child = child->next) simplify_expressions(child, report);
    for (child = node->properties; child; child = child->next) simplify_expressions(child, report);
    for (child = node->children; child; child = child->next) simplify_expressions(child, report);

    if (node->kind == ELX_AST_BINARY) {
        if ((node->operator == ELX_TOKEN_PLUS || node->operator == ELX_TOKEN_MINUS) && is_zero(node->right)) {
            replace_with_child(node, node->left, true);
            ++report->combined_instructions;
        } else if (node->operator == ELX_TOKEN_PLUS && is_zero(node->left)) {
            replace_with_child(node, node->right, false);
            ++report->combined_instructions;
        } else if ((node->operator == ELX_TOKEN_STAR || node->operator == ELX_TOKEN_SLASH) && is_one(node->right)) {
            replace_with_child(node, node->left, true);
            ++report->combined_instructions;
        } else if (node->operator == ELX_TOKEN_STAR && is_one(node->left)) {
            replace_with_child(node, node->right, false);
            ++report->combined_instructions;
        }
    } else if (node->kind == ELX_AST_UNARY && node->operator == ELX_TOKEN_BANG &&
               node->left && node->left->kind == ELX_AST_UNARY && node->left->operator == ELX_TOKEN_BANG) {
        elx_ast_t *inner = node->left;
        elx_ast_t *replacement = inner->left;
        if (replacement) {
            inner->left = NULL;
            node->left = NULL;
            elx_ast_destroy(inner);
            steal_ast_contents(node, replacement);
            ++report->combined_instructions;
        }
    }
}

static void trim_unreachable(elx_ast_t *node, elx_optimization_report_t *report) {
    elx_ast_t **cursor;
    elx_ast_t *statement;
    bool terminal;
    if (!node) return;
    if (node->kind == ELX_AST_BLOCK) {
        cursor = &node->next;
        terminal = false;
        while (*cursor) {
            statement = *cursor;
            if (terminal) {
                *cursor = statement->next;
                statement->next = NULL;
                elx_ast_destroy(statement);
                ++report->eliminated_dead_code;
                continue;
            }
            if (statement->kind == ELX_AST_RETURN || statement->kind == ELX_AST_BREAK || statement->kind == ELX_AST_CONTINUE) {
                terminal = true;
            }
            cursor = &statement->next;
        }
    }
    trim_unreachable(node->left, report);
    trim_unreachable(node->right, report);
    trim_unreachable(node->third, report);
    trim_unreachable(node->body, report);
    trim_unreachable(node->else_body, report);
    trim_unreachable(node->condition, report);
    trim_unreachable(node->init, report);
    trim_unreachable(node->step, report);
    trim_unreachable(node->target, report);
    trim_unreachable(node->value, report);
    trim_unreachable(node->callee, report);
    trim_unreachable(node->args, report);
    trim_unreachable(node->object, report);
    trim_unreachable(node->items, report);
    trim_unreachable(node->properties, report);
    trim_unreachable(node->children, report);
}

static void mark_tail_calls(elx_ast_t *node, elx_optimization_report_t *report) {
    elx_ast_t *child;
    if (!node) return;
    if (node->kind == ELX_AST_FUNCTION && node->body) {
        for (child = node->body->next; child; child = child->next) {
            if (child->kind == ELX_AST_RETURN && child->value && child->value->kind == ELX_AST_CALL) {
                child->value->tail_call = true;
                ++report->tail_calls;
            }
        }
    }
    for (child = node->left; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->right; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->third; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->body; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->else_body; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->condition; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->init; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->step; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->target; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->value; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->callee; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->args; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->object; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->items; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->properties; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->children; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->next; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->items; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->properties; child; child = child->next) mark_tail_calls(child, report);
    for (child = node->children; child; child = child->next) mark_tail_calls(child, report);
}

static void mark_stack_allocations(elx_ast_t *node, elx_optimization_report_t *report) {
    elx_ast_t *child;
    size_t count;
    if (!node) return;
    if ((node->kind == ELX_AST_STRUCT_LITERAL || node->kind == ELX_AST_ARRAY) &&
        (count = elx_ast_count(node->properties ? node->properties : node->items)) <= 8U) {
        node->stack_allocated = true;
        ++report->stack_allocations;
    }
    for (child = node->left; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->right; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->third; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->body; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->else_body; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->condition; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->init; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->step; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->target; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->value; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->callee; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->args; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->object; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->items; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->properties; child; child = child->next) mark_stack_allocations(child, report);
    for (child = node->children; child; child = child->next) mark_stack_allocations(child, report);
}

static void fold_constants(elx_ast_t *node, elx_optimization_report_t *report) {
    elx_ast_t *child;
    if (!node) return;
    if (node->kind == ELX_AST_BINARY && is_constant(node->left) && is_constant(node->right)) {
        if (node->left->kind == ELX_AST_INT && node->right->kind == ELX_AST_INT) {
            uint64_t left = node->left->integer;
            uint64_t right = node->right->integer;
            uint64_t result = 0U;
            switch (node->operator) {
                case ELX_TOKEN_PLUS: result = left + right; break;
                case ELX_TOKEN_MINUS: result = left - right; break;
                case ELX_TOKEN_STAR: result = left * right; break;
                case ELX_TOKEN_SLASH: result = right ? left / right : 0U; break;
                case ELX_TOKEN_PERCENT: result = right ? left % right : 0U; break;
                case ELX_TOKEN_LT: result = left < right; break;
                case ELX_TOKEN_LT_EQ: result = left <= right; break;
                case ELX_TOKEN_GT: result = left > right; break;
                case ELX_TOKEN_GT_EQ: result = left >= right; break;
                case ELX_TOKEN_EQ:
                case ELX_TOKEN_EQ_EQ: result = left == right; break;
                case ELX_TOKEN_BANG_EQ: result = left != right; break;
                case ELX_TOKEN_AND_AND: result = left && right; break;
                case ELX_TOKEN_OR_OR: result = left || right; break;
                default: break;
            }
            node->kind = ELX_AST_INT;
            node->integer = result;
            ++report->folded_constants;
        } else if (node->left->kind == ELX_AST_FLOAT && node->right->kind == ELX_AST_FLOAT) {
            double left = node->left->real;
            double right = node->right->real;
            double result = 0.0;
            switch (node->operator) {
                case ELX_TOKEN_PLUS: result = left + right; break;
                case ELX_TOKEN_MINUS: result = left - right; break;
                case ELX_TOKEN_STAR: result = left * right; break;
                case ELX_TOKEN_SLASH: result = right ? left / right : 0.0; break;
                case ELX_TOKEN_LT: result = left < right; break;
                case ELX_TOKEN_LT_EQ: result = left <= right; break;
                case ELX_TOKEN_GT: result = left > right; break;
                case ELX_TOKEN_GT_EQ: result = left >= right; break;
                case ELX_TOKEN_EQ:
                case ELX_TOKEN_EQ_EQ: result = left == right; break;
                case ELX_TOKEN_BANG_EQ: result = left != right; break;
                case ELX_TOKEN_AND_AND: result = left && right; break;
                case ELX_TOKEN_OR_OR: result = left || right; break;
                default: break;
            }
            node->kind = node->operator == ELX_TOKEN_AND_AND || node->operator == ELX_TOKEN_OR_OR
                ? ELX_AST_INT : ELX_AST_FLOAT;
            node->real = result;
            ++report->folded_constants;
        }
    }
    if (node->kind == ELX_AST_UNARY && node->left && is_constant(node->left)) {
        if (node->operator == ELX_TOKEN_MINUS) {
            if (node->left->kind == ELX_AST_INT) {
                node->left->integer = -((int64_t)node->left->integer);
                steal_ast_contents(node, node->left);
                node->left = NULL;
                ++report->folded_constants;
            } else if (node->left->kind == ELX_AST_FLOAT) {
                node->left->real = -node->left->real;
                steal_ast_contents(node, node->left);
                node->left = NULL;
                ++report->folded_constants;
            }
        } else if (node->operator == ELX_TOKEN_BANG) {
            if (node->left->kind == ELX_AST_INT) {
                node->left->integer = node->left->integer ? 0U : 1U;
                steal_ast_contents(node, node->left);
                node->left = NULL;
                ++report->folded_constants;
            } else if (node->left->kind == ELX_AST_BOOL) {
                node->kind = ELX_AST_BOOL;
                node->boolean = !node->left->boolean;
                ++report->folded_constants;
            }
        }
    }
    for (child = node->left; child; child = child->next) fold_constants(child, report);
    for (child = node->right; child; child = child->next) fold_constants(child, report);
    for (child = node->third; child; child = child->next) fold_constants(child, report);
    for (child = node->body; child; child = child->next) fold_constants(child, report);
    for (child = node->else_body; child; child = child->next) fold_constants(child, report);
    for (child = node->condition; child; child = child->next) fold_constants(child, report);
    for (child = node->init; child; child = child->next) fold_constants(child, report);
    for (child = node->step; child; child = child->next) fold_constants(child, report);
    for (child = node->target; child; child = child->next) fold_constants(child, report);
    for (child = node->value; child; child = child->next) fold_constants(child, report);
    for (child = node->callee; child; child = child->next) fold_constants(child, report);
    for (child = node->args; child; child = child->next) fold_constants(child, report);
    for (child = node->object; child; child = child->next) fold_constants(child, report);
    for (child = node->items; child; child = child->next) fold_constants(child, report);
    for (child = node->properties; child; child = child->next) fold_constants(child, report);
    for (child = node->children; child; child = child->next) fold_constants(child, report);
}

static void infer_layout_hints(elx_ast_t *node) {
    elx_ast_t *child;
    if (!node) return;
    if (node->kind == ELX_AST_WIDGET && node->name) {
        if (strcmp(node->name, "Column") == 0) {
            node->layout_hint = copy_string("flex-column");
        } else if (strcmp(node->name, "Row") == 0) {
            node->layout_hint = copy_string("flex-row");
        } else if (strcmp(node->name, "Grid") == 0) {
            node->layout_hint = copy_string("grid");
        } else if (strcmp(node->name, "Stack") == 0) {
            node->layout_hint = copy_string("stack");
        } else if (strcmp(node->name, "Window") == 0) {
            node->layout_hint = copy_string("window");
        }
    }
    for (child = node->children; child; child = child->next) infer_layout_hints(child);
    for (child = node->items; child; child = child->next) infer_layout_hints(child);
    for (child = node->body; child; child = child->next) infer_layout_hints(child);
    for (child = node->else_body; child; child = child->next) infer_layout_hints(child);
    for (child = node->condition; child; child = child->next) infer_layout_hints(child);
    for (child = node->value; child; child = child->next) infer_layout_hints(child);
    for (child = node->left; child; child = child->next) infer_layout_hints(child);
    for (child = node->right; child; child = child->next) infer_layout_hints(child);
    for (child = node->third; child; child = child->next) infer_layout_hints(child);
    for (child = node->properties; child; child = child->next) infer_layout_hints(child);
}

static void mark_reactive(elx_ast_t *node, elx_optimization_report_t *report) {
    elx_ast_t *child;
    if (!node) return;
    if (node->kind == ELX_AST_UI_PROPERTY && node->name && node->value) {
        if (node->value->kind == ELX_AST_CALL || node->value->kind == ELX_AST_MEMBER) {
            node->reactive = true;
            ++report->reactive_edges;
        }
    }
    for (child = node->children; child; child = child->next) mark_reactive(child, report);
    for (child = node->items; child; child = child->next) mark_reactive(child, report);
    for (child = node->body; child; child = child->next) mark_reactive(child, report);
    for (child = node->else_body; child; child = child->next) mark_reactive(child, report);
    for (child = node->condition; child; child = child->next) mark_reactive(child, report);
    for (child = node->value; child; child = child->next) mark_reactive(child, report);
    for (child = node->left; child; child = child->next) mark_reactive(child, report);
    for (child = node->right; child; child = child->next) mark_reactive(child, report);
    for (child = node->third; child; child = child->next) mark_reactive(child, report);
    for (child = node->properties; child; child = child->next) mark_reactive(child, report);
}

static void collect_update_order(const elx_ast_t *node, elx_optimization_report_t *report) {
    const elx_ast_t *child;
    if (!node) return;
    if (node->kind == ELX_AST_STATE && node->name) {
        append_update_order(report, node->name);
    }
    if (node->kind == ELX_AST_WIDGET && node->name) {
        append_update_order(report, node->name);
    }
    for (child = node->next; child; child = child->next) collect_update_order(child, report);
    for (child = node->children; child; child = child->next) collect_update_order(child, report);
    for (child = node->items; child; child = child->next) collect_update_order(child, report);
    for (child = node->body; child; child = child->next) collect_update_order(child, report);
    for (child = node->else_body; child; child = child->next) collect_update_order(child, report);
    for (child = node->condition; child; child = child->next) collect_update_order(child, report);
    for (child = node->value; child; child = child->next) collect_update_order(child, report);
    for (child = node->left; child; child = child->next) collect_update_order(child, report);
    for (child = node->right; child; child = child->next) collect_update_order(child, report);
    for (child = node->third; child; child = child->next) collect_update_order(child, report);
    for (child = node->properties; child; child = child->next) collect_update_order(child, report);
}

static void eliminate_unused(elx_ast_t *node, elx_optimization_report_t *report) {
    elx_ast_t **cursor;
    elx_ast_t *child;
    if (!node) return;
    cursor = &node->children;
    while (*cursor) {
        child = *cursor;
        if (!child->used && child->kind == ELX_AST_WIDGET) {
            *cursor = child->next;
            child->next = NULL;
            elx_ast_destroy(child);
            ++report->removed_unused_widgets;
        } else {
            cursor = &child->next;
        }
    }
    for (child = node->children; child; child = child->next) eliminate_unused(child, report);
    for (child = node->items; child; child = child->next) eliminate_unused(child, report);
    for (child = node->body; child; child = child->next) eliminate_unused(child, report);
    for (child = node->else_body; child; child = child->next) eliminate_unused(child, report);
    for (child = node->condition; child; child = child->next) eliminate_unused(child, report);
    for (child = node->value; child; child = child->next) eliminate_unused(child, report);
    for (child = node->left; child; child = child->next) eliminate_unused(child, report);
    for (child = node->right; child; child = child->next) eliminate_unused(child, report);
    for (child = node->third; child; child = child->next) eliminate_unused(child, report);
    for (child = node->properties; child; child = child->next) eliminate_unused(child, report);
}

void elx_optimize(elx_ast_t *program, elx_optimization_report_t *report) {
    if (!program || !report) return;
    memset(report, 0, sizeof(*report));
    count_nodes(program, report);
    fold_constants(program, report);
    simplify_expressions(program, report);
    trim_unreachable(program, report);
    mark_tail_calls(program, report);
    mark_stack_allocations(program, report);
    infer_layout_hints(program);
    mark_reactive(program, report);
    collect_update_order(program, report);
    eliminate_unused(program, report);
}
