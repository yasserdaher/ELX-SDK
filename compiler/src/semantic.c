#define _GNU_SOURCE
#include "elx.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct elx_resolved_module {
    char *name;
    char *path;
    elx_ast_t *ast;
    elx_scope_t *global;
    struct elx_resolved_module *next;
} elx_resolved_module_t;

typedef struct {
    char *current_file;
    elx_resolved_module_t *modules;
} module_context_t;

static module_context_t *module_context_new(void) {
    return (module_context_t *)calloc(1U, sizeof(module_context_t));
}

typedef struct {
    elx_scope_t *global;
    elx_scope_t *current;
    size_t scope_depth;
    elx_diagnostic_list_t *diagnostics;
    bool has_errors;
    module_context_t *module_ctx;
} semantic_context_t;

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

static void semantic_error(semantic_context_t *context, int code, const char *message, elx_location_t location) {
    elx_diagnostic_add(context->diagnostics, code, message, location);
    context->has_errors = true;
}

static elx_scope_t *scope_new(elx_scope_t *parent) {
    elx_scope_t *scope;
    scope = (elx_scope_t *)calloc(1U, sizeof(*scope));
    if (!scope) return NULL;
    scope->parent = parent;
    return scope;
}

static void scope_push(semantic_context_t *context) {
    elx_scope_t *scope = scope_new(context->current);
    if (scope) {
        ++context->scope_depth;
        context->current = scope;
    }
}

static void scope_pop(semantic_context_t *context) {
    elx_scope_t *scope = context->current;
    size_t i;
    if (!scope || !scope->parent) return;
    context->current = scope->parent;
    if (context->scope_depth > 0U) --context->scope_depth;
    for (i = 0U; i < scope->count; ++i) {
        free(scope->symbols[i].name);
        elx_type_destroy(scope->symbols[i].type);
    }
    free(scope->symbols);
    free(scope);
}

static elx_symbol_t *scope_lookup(elx_scope_t *scope, const char *name) {
    size_t i;
    while (scope) {
        for (i = 0U; i < scope->count; ++i) {
            if (scope->symbols[i].name && name && strcmp(scope->symbols[i].name, name) == 0) {
                return &scope->symbols[i];
            }
        }
        scope = scope->parent;
    }
    return NULL;
}

static void module_context_free(module_context_t *ctx);
static void scope_push(semantic_context_t *context);
static void scope_pop(semantic_context_t *context);
static elx_symbol_t *scope_lookup(elx_scope_t *scope, const char *name);
static bool scope_define(semantic_context_t *context, const char *name, elx_type_t *type, bool mutable_value, bool public_value, elx_location_t location);
static elx_type_t *type_from_ast(const elx_ast_t *node);
static void analyze_expression(semantic_context_t *context, const elx_ast_t *expression);
static void analyze_function(semantic_context_t *context, const elx_ast_t *function);
static void analyze_state(semantic_context_t *context, const elx_ast_t *state);
static void analyze_block(semantic_context_t *context, const elx_ast_t *block);
static void analyze_program(semantic_context_t *context, const elx_ast_t *program);
static elx_resolved_module_t *load_and_resolve_module(semantic_context_t *context, const char *path);
static elx_resolved_module_t *find_module_by_path(module_context_t *ctx, const char *path) {
    elx_resolved_module_t *m = ctx ? ctx->modules : NULL;
    while (m) {
        if (m->path && path && strcmp(m->path, path) == 0) return m;
        m = m->next;
    }
    return NULL;
}

static void module_context_free(module_context_t *ctx) {
    elx_resolved_module_t *m;
    if (!ctx) return;
    free(ctx->current_file);
    m = ctx->modules;
    while (m) {
        elx_resolved_module_t *next = m->next;
        free(m->name);
        free(m->path);
        if (m->ast) elx_ast_destroy(m->ast);
        if (m->global) {
            for (size_t i = 0U; i < m->global->count; ++i) {
                free(m->global->symbols[i].name);
                elx_type_destroy(m->global->symbols[i].type);
            }
            free(m->global->symbols);
            free(m->global);
        }
        free(m);
        m = next;
    }
    free(ctx);
}

static char *resolve_import_path(const char *import_path, const char *current_file) {
    char *result;
    const char *slash;
    size_t dir_len;
    size_t path_len;
    if (!import_path) return NULL;
    if (strchr(import_path, '/') != NULL) {
        result = copy_string(import_path);
        return result;
    }
    if (current_file && (slash = strrchr(current_file, '/')) != NULL) {
        dir_len = (size_t)(slash - current_file) + 1;
        path_len = dir_len + strlen(import_path) + 1;
        result = (char *)calloc(path_len, 1);
        if (result) {
            memcpy(result, current_file, dir_len);
            strcpy(result + dir_len, import_path);
            return result;
        }
    }
    path_len = strlen(import_path) + 1;
    result = (char *)calloc(path_len, 1);
    if (result) strcpy(result, import_path);
    return result;
}

static elx_ast_t *load_module_ast(const char *path, elx_diagnostic_list_t *diagnostics) {
    FILE *file;
    long length;
    char *source;
    elx_ast_t *ast;
    if (!path || !diagnostics) return NULL;
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) { fclose(file); return NULL; }
    length = ftell(file);
    if (length < 0) { fclose(file); return NULL; }
    source = (char *)malloc((size_t)length + 1U);
    if (!source) { fclose(file); return NULL; }
    if (fseek(file, 0, SEEK_SET) != 0) { free(source); fclose(file); return NULL; }
    if (length > 0 && fread(source, 1U, (size_t)length, file) != (size_t)length) {
        free(source); fclose(file); return NULL;
    }
    source[length] = '\0';
    fclose(file);
    ast = elx_parse(source, (size_t)length, diagnostics);
    free(source);
    return ast;
}

static elx_resolved_module_t *load_and_resolve_module(semantic_context_t *context, const char *path);

static elx_resolved_module_t *load_and_resolve_module(semantic_context_t *context, const char *path) {
    elx_resolved_module_t *resolved;
    elx_ast_t *ast;
    elx_diagnostic_list_t *mod_diag;
    elx_scope_t *mod_scope;
    char *full_path;
    char *mod_name;
    char *mod_name_orig;
    char *slash;
    char *dot;
    semantic_context_t mod_context;
    elx_ast_t *decl;
    if (!path) return NULL;
    resolved = find_module_by_path(context->module_ctx, path);
    if (resolved) return resolved;
    full_path = resolve_import_path(path, context->module_ctx->current_file);
    if (!full_path) return NULL;
    mod_diag = (elx_diagnostic_list_t *)calloc(1, sizeof(*mod_diag));
    if (!mod_diag) { free(full_path); return NULL; }
    elx_diagnostics_init(mod_diag);
    ast = load_module_ast(full_path, mod_diag);
    if (!ast || mod_diag->has_errors) {
        if (mod_diag->count > 0) {
            for (size_t d = 0; d < mod_diag->count; ++d) {
                elx_diagnostic_add(context->diagnostics, mod_diag->items[d].code,
                    mod_diag->items[d].message, mod_diag->items[d].location);
            }
        }
        elx_diagnostics_destroy(mod_diag);
        free(mod_diag);
        free(full_path);
        return NULL;
    }
    mod_scope = (elx_scope_t *)calloc(1, sizeof(*mod_scope));
    if (!mod_scope) {
        elx_ast_destroy(ast);
        elx_diagnostics_destroy(mod_diag);
        free(mod_diag);
        free(full_path);
        return NULL;
    }
    resolved = (elx_resolved_module_t *)calloc(1, sizeof(*resolved));
    if (!resolved) {
        elx_ast_destroy(ast);
        free(mod_scope);
        elx_diagnostics_destroy(mod_diag);
        free(mod_diag);
        free(full_path);
        return NULL;
    }
    mod_name_orig = copy_string(path);
    mod_name = mod_name_orig;
    slash = strrchr(mod_name, '/');
    if (slash) mod_name = slash + 1;
    dot = strrchr(mod_name, '.');
    if (dot) *dot = '\0';
    resolved->name = strdup(mod_name);
    free(mod_name_orig);
    resolved->path = full_path;
    resolved->ast = ast;
    resolved->global = mod_scope;
    resolved->next = NULL;
    elx_resolved_module_t **cursor = &context->module_ctx->modules;
    while (*cursor) cursor = &(*cursor)->next;
    *cursor = resolved;
    mod_context.global = mod_scope;
    mod_context.current = mod_scope;
    mod_context.scope_depth = 0;
    mod_context.diagnostics = mod_diag;
    mod_context.module_ctx = context->module_ctx;
    for (decl = ast; decl; decl = decl->next) {
        if (decl->kind == ELX_AST_FUNCTION) {
            analyze_function(&mod_context, decl);
        } else if (decl->kind == ELX_AST_STATE) {
            analyze_state(&mod_context, decl);
        } else if (decl->kind == ELX_AST_CONST || decl->kind == ELX_AST_VAR) {
            scope_define(&mod_context, decl->name, type_from_ast(decl),
                decl->mutable_value, true, decl->location);
            analyze_expression(&mod_context, decl->value);
        } else if (decl->kind == ELX_AST_IMPORT) {
            load_and_resolve_module(&mod_context, decl->import_path);
        }
    }
    elx_diagnostics_destroy(mod_diag);
    free(mod_diag);
    return resolved;
}

static bool scope_define(semantic_context_t *context, const char *name, elx_type_t *type, bool mutable_value, bool public_value, elx_location_t location) {
    elx_symbol_t *existing;
    elx_symbol_t *symbols;
    elx_scope_t *scope = context->current;
    existing = scope_lookup(scope, name);
    if (existing) {
        semantic_error(context, 302, "duplicate symbol", location);
        return false;
    }
    if (scope->count == scope->capacity) {
        size_t capacity = scope->capacity ? scope->capacity * 2U : 16U;
        symbols = (elx_symbol_t *)realloc(scope->symbols, capacity * sizeof(*symbols));
        if (!symbols) return false;
        scope->symbols = symbols;
        scope->capacity = capacity;
    }
    scope->symbols[scope->count].name = copy_string(name);
    scope->symbols[scope->count].type = type;
    scope->symbols[scope->count].mutable_value = mutable_value;
    scope->symbols[scope->count].public_value = public_value;
    scope->symbols[scope->count].defined = true;
    scope->symbols[scope->count].scope_depth = context->scope_depth;
    ++scope->count;
    return true;
}

static bool is_builtin_type(const char *name) {
    return strcmp(name, "Int") == 0 || strcmp(name, "Float") == 0 || strcmp(name, "Bool") == 0 ||
           strcmp(name, "String") == 0 || strcmp(name, "StringView") == 0 || strcmp(name, "Ptr") == 0 ||
           strcmp(name, "Void") == 0 || strcmp(name, "Widget") == 0;
}

static elx_type_t *type_from_ast(const elx_ast_t *node) {
    if (!node || !node->type) return elx_type_new(ELX_TYPE_INVALID);
    return elx_type_clone(node->type);
}

static elx_type_t *expression_type(semantic_context_t *context, const elx_ast_t *node) {
    elx_symbol_t *symbol;
    if (!node) return elx_type_new(ELX_TYPE_INVALID);
    switch (node->kind) {
        case ELX_AST_INT: return elx_type_new(ELX_TYPE_INT);
        case ELX_AST_FLOAT: return elx_type_new(ELX_TYPE_FLOAT);
        case ELX_AST_STRING: return elx_type_new(ELX_TYPE_STRING);
        case ELX_AST_BOOL: return elx_type_new(ELX_TYPE_BOOL);
        case ELX_AST_NIL: return elx_type_new(ELX_TYPE_INVALID);
        case ELX_AST_IDENT:
            symbol = scope_lookup(context->global, node->name);
            if (!symbol) symbol = scope_lookup(context->current, node->name);
            if (!symbol) {
                semantic_error(context, 301, "missing symbol", node->location);
                return elx_type_new(ELX_TYPE_INVALID);
            }
            return elx_type_clone(symbol->type);
        case ELX_AST_BINARY: {
            elx_type_t *left = expression_type(context, node->left);
            elx_type_t *right = expression_type(context, node->right);
            if (left->kind == ELX_TYPE_INVALID || right->kind == ELX_TYPE_INVALID) {
                elx_type_destroy(left);
                elx_type_destroy(right);
                return elx_type_new(ELX_TYPE_INVALID);
            }
            if (!elx_type_equal(left, right)) {
                semantic_error(context, 303, "type mismatch in binary expression", node->location);
                elx_type_destroy(left);
                elx_type_destroy(right);
                return elx_type_new(ELX_TYPE_INVALID);
            }
            elx_type_destroy(right);
            return left;
        }
        case ELX_AST_UNARY:
            return expression_type(context, node->left);
        case ELX_AST_ASSIGN:
            return expression_type(context, node->right);
        case ELX_AST_CALL: {
            elx_symbol_t *sym;
            if (node->func_name) {
                sym = scope_lookup(context->global, node->func_name);
                if (sym && sym->type && sym->type->kind == ELX_TYPE_FN && sym->type->return_type) {
                    return elx_type_clone(sym->type->return_type);
                }
            }
            if (node->callee && node->callee->kind == ELX_AST_IDENT && node->callee->name) {
                sym = scope_lookup(context->global, node->callee->name);
                if (sym && sym->type && sym->type->kind == ELX_TYPE_FN && sym->type->return_type) {
                    return elx_type_clone(sym->type->return_type);
                }
            }
            return elx_type_new(ELX_TYPE_INVALID);
        }
        case ELX_AST_MEMBER:
            return elx_type_new(ELX_TYPE_INVALID);
        case ELX_AST_STRUCT_LITERAL:
            return elx_type_named(node->name);
        case ELX_AST_ARRAY:
            return elx_type_new(ELX_TYPE_ARRAY);
        case ELX_AST_LAMBDA:
            return elx_type_new(ELX_TYPE_FN);
        case ELX_AST_UI_PROPERTY:
            return expression_type(context, node->value);
        case ELX_AST_UI_EVENT:
            return expression_type(context, node->value);
        default:
            return elx_type_new(ELX_TYPE_INVALID);
    }
}

static void analyze_expression(semantic_context_t *context, const elx_ast_t *expression) {
    elx_type_t *type;
    if (!expression) return;
    type = expression_type(context, expression);
    elx_type_destroy(type);
}

static void analyze_block(semantic_context_t *context, const elx_ast_t *block);

static void analyze_statement(semantic_context_t *context, const elx_ast_t *statement) {
    elx_ast_t *child;
    if (!statement) return;
    switch (statement->kind) {
        case ELX_AST_VAR:
        case ELX_AST_CONST:
            scope_define(context, statement->name, type_from_ast(statement), statement->mutable_value, false, statement->location);
            analyze_expression(context, statement->value);
            break;
        case ELX_AST_BLOCK:
            analyze_block(context, statement);
            break;
        case ELX_AST_IF:
            analyze_expression(context, statement->condition);
            analyze_block(context, statement->body);
            analyze_block(context, statement->else_body);
            break;
        case ELX_AST_MATCH:
            analyze_expression(context, statement->condition);
            for (child = statement->items; child; child = child->next) {
                analyze_expression(context, child->left);
                analyze_expression(context, child->right);
            }
            break;
        case ELX_AST_FOR:
            analyze_expression(context, statement->target);
            analyze_expression(context, statement->condition);
            analyze_block(context, statement->body);
            break;
        case ELX_AST_WHILE:
        case ELX_AST_LOOP:
            analyze_expression(context, statement->condition);
            analyze_block(context, statement->body);
            break;
        case ELX_AST_RETURN:
            analyze_expression(context, statement->value);
            break;
        case ELX_AST_BREAK:
        case ELX_AST_CONTINUE:
            break;
        case ELX_AST_EXPR_STMT:
            analyze_expression(context, statement->value);
            break;
        case ELX_AST_ASSIGN:
            analyze_expression(context, statement->left);
            analyze_expression(context, statement->right);
            break;
        default:
            analyze_expression(context, statement->value);
            break;
    }
}

static void analyze_block(semantic_context_t *context, const elx_ast_t *block) {
    elx_ast_t *statement;
    if (!block) return;
    scope_push(context);
    for (statement = block->next; statement; statement = statement->next) {
        analyze_statement(context, statement);
    }
    scope_pop(context);
}

static void analyze_function(semantic_context_t *context, const elx_ast_t *function) {
    elx_ast_t *parameter;
    if (!function) return;
    scope_define(context, function->name, elx_type_fn(function->type, (elx_ast_t *)function->args), false, true, function->location);
    scope_push(context);
    for (parameter = function->args; parameter; parameter = parameter->next) {
        if (parameter->type && parameter->type->kind == ELX_TYPE_NAMED && !is_builtin_type(parameter->type->name) &&
            !scope_lookup(context->global, parameter->type->name)) {
            semantic_error(context, 300, "unknown type", parameter->location);
        }
        scope_define(context, parameter->name, type_from_ast(parameter), true, false, parameter->location);
    }
    analyze_block(context, function->body);
    scope_pop(context);
}

static void analyze_state(semantic_context_t *context, const elx_ast_t *state) {
    elx_ast_t *field;
    if (!state) return;
    scope_define(context, state->name, elx_type_named(state->name), false, true, state->location);
    for (field = state->items; field; field = field->next) {
        if (field->type && field->type->kind == ELX_TYPE_NAMED && !is_builtin_type(field->type->name) &&
            !scope_lookup(context->global, field->type->name)) {
            semantic_error(context, 300, "unknown type", field->location);
        }
        scope_define(context, field->name, type_from_ast(field), true, false, field->location);
        analyze_expression(context, field->value);
    }
}

static void analyze_permissions(semantic_context_t *context, const elx_ast_t *permissions) {
    elx_ast_t *property;
    if (!permissions) return;
    for (property = permissions->properties; property; property = property->next) {
        analyze_expression(context, property->value);
    }
}

static void analyze_widget(semantic_context_t *context, const elx_ast_t *widget) {
    elx_ast_t *property;
    elx_ast_t *child;
    if (!widget) return;
    for (property = widget->properties; property; property = property->next) {
        if (property->kind == ELX_AST_UI_EVENT) {
            analyze_block(context, property->value);
        } else {
            analyze_expression(context, property->value);
        }
    }
    for (child = widget->children; child; child = child->next) {
        analyze_widget(context, child);
    }
}

static void process_import(semantic_context_t *context, const elx_ast_t *import_node) {
    elx_resolved_module_t *resolved;
    elx_scope_t *mod_scope;
    size_t i;
    const char *alias;
    if (!import_node || !import_node->import_path) return;
    if (!context->module_ctx) return;
    /* Built-in standard library modules (std.*) are implemented in C and
       do not have corresponding .elx source files.  Skip file loading for
       these modules so that import does not fail. */
    if (strncmp(import_node->import_path, "std.", 4) == 0 ||
        strcmp(import_node->import_path, "std") == 0) {
        return;
    }
    resolved = load_and_resolve_module(context, import_node->import_path);
    if (!resolved) {
        semantic_error(context, 310, "cannot resolve import", import_node->location);
        return;
    }
    alias = import_node->alias ? import_node->alias : resolved->name;
    mod_scope = resolved->global;
    if (!mod_scope) return;
    for (i = 0U; i < mod_scope->count; ++i) {
        if (mod_scope->symbols[i].public_value) {
            char sym_name[512];
            size_t name_len = strlen(alias) + 1 + strlen(mod_scope->symbols[i].name) + 1;
            snprintf(sym_name, name_len, "%s.%s", alias, mod_scope->symbols[i].name);
            if (!scope_lookup(context->current, sym_name)) {
                scope_define(context, strdup(sym_name), elx_type_clone(mod_scope->symbols[i].type),
                    false, true, import_node->location);
            }
        }
    }
}

static void process_export(semantic_context_t *context, const elx_ast_t *export_node) {
    if (!export_node) return;
    if (export_node->value) {
        elx_ast_t *decl = export_node->value;
        if (decl->kind == ELX_AST_FUNCTION) {
            decl->public_value = true;
            analyze_function(context, decl);
        } else if (decl->kind == ELX_AST_CONST || decl->kind == ELX_AST_VAR) {
            decl->public_value = true;
            scope_define(context, decl->name, type_from_ast(decl), decl->mutable_value, true, decl->location);
            analyze_expression(context, decl->value);
        }
    }
}

static void process_module(semantic_context_t *context, const elx_ast_t *module_node) {
    if (!module_node || !module_node->module_name) return;
    if (context->module_ctx->current_file) free(context->module_ctx->current_file);
    context->module_ctx->current_file = copy_string(module_node->module_name);
}

static void analyze_program(semantic_context_t *context, const elx_ast_t *program) {
    const elx_ast_t *declaration;
    for (declaration = program; declaration; declaration = declaration->next) {
        switch (declaration->kind) {
            case ELX_AST_IMPORT:
                process_import(context, declaration);
                break;
            case ELX_AST_EXPORT:
                process_export(context, declaration);
                break;
            case ELX_AST_MODULE:
                process_module(context, declaration);
                break;
            case ELX_AST_FUNCTION:
                analyze_function(context, declaration);
                break;
            case ELX_AST_STATE:
                analyze_state(context, declaration);
                break;
            case ELX_AST_PERMISSIONS:
                analyze_permissions(context, declaration);
                break;
            case ELX_AST_WIDGET:
                analyze_widget(context, declaration);
                break;
            default:
                break;
        }
    }
}

static void register_builtins(semantic_context_t *context) {
    elx_type_t *fn_type;
    elx_type_t *param;
    elx_ast_t *params;
    elx_type_t *ret_int = elx_type_new(ELX_TYPE_INT);
    elx_type_t *ret_str = elx_type_new(ELX_TYPE_STRING);
    elx_type_t *ret_num = elx_type_named("Float");
    elx_type_t *ret_void = elx_type_new(ELX_TYPE_INVALID);
    (void)ret_str;

    {
        params = NULL;
        param = elx_type_new(ELX_TYPE_STRING);
        params = elx_ast_new(ELX_AST_IDENT, (elx_location_t){0, 0, 0});
        params->name = strdup("s");
        params->type = param;
        fn_type = elx_type_fn(ret_int, params);
        fn_type->kind = ELX_TYPE_FN;
        scope_define(context, "str.len", fn_type, false, true, (elx_location_t){0, 0, 0});
    }

    {
        params = NULL;
        param = elx_type_new(ELX_TYPE_STRING);
        params = elx_ast_new(ELX_AST_IDENT, (elx_location_t){0, 0, 0});
        params->name = strdup("s");
        params->type = param;
        fn_type = elx_type_fn(ret_void, params);
        scope_define(context, "print", fn_type, false, true, (elx_location_t){0, 0, 0});
    }

    {
        params = NULL;
        param = elx_type_new(ELX_TYPE_INT);
        params = elx_ast_new(ELX_AST_IDENT, (elx_location_t){0, 0, 0});
        params->name = strdup("x");
        params->type = param;
        fn_type = elx_type_fn(ret_num, params);
        fn_type->kind = ELX_TYPE_FN;
        scope_define(context, "math.sqrt", fn_type, false, true, (elx_location_t){0, 0, 0});
    }

    elx_type_destroy(ret_int);
    elx_type_destroy(ret_str);
    elx_type_destroy(ret_num);
    (void)ret_void;
}

bool elx_semantic_analyze(elx_ast_t *program, elx_diagnostic_list_t *diagnostics) {
    semantic_context_t context;
    elx_scope_t *scope;
    module_context_t *module_ctx;
    if (!program || !diagnostics) return false;
    module_ctx = module_context_new();
    if (!module_ctx) return false;
    context.diagnostics = diagnostics;
    context.has_errors = false;
    context.scope_depth = 0U;
    context.module_ctx = module_ctx;
    scope = scope_new(NULL);
    if (!scope) { module_context_free(module_ctx); return false; }
    context.global = scope;
    context.current = scope;
    register_builtins(&context);
    analyze_program(&context, program);
    scope_pop(&context);
    free(scope);
    module_context_free(module_ctx);
    return !context.has_errors;
}
