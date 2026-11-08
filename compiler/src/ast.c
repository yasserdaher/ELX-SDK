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

void elx_diagnostics_init(elx_diagnostic_list_t *list) {
    if (!list) return;
    memset(list, 0, sizeof(*list));
}

void elx_diagnostics_destroy(elx_diagnostic_list_t *list) {
    size_t i;
    if (!list) return;
    for (i = 0U; i < list->count; ++i) {
        free((char *)list->items[i].message);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

void elx_diagnostic_add(elx_diagnostic_list_t *list, int code, const char *message, elx_location_t location) {
    elx_diagnostic_t *item;
    if (!list || !message) return;
    if (list->count == list->capacity) {
        size_t capacity = list->capacity ? list->capacity * 2U : 16U;
        elx_diagnostic_t *items = (elx_diagnostic_t *)realloc(list->items, capacity * sizeof(*items));
        if (!items) return;
        list->items = items;
        list->capacity = capacity;
    }
    item = &list->items[list->count++];
    item->code = code;
    item->message = copy_string(message);
    item->location = location;
    if (code >= 100 && code < 400) list->has_errors = true;
}

void elx_diagnostics_print(const elx_diagnostic_list_t *list, const char *source_name, FILE *stream) {
    size_t i;
    if (!list || !stream) return;
    for (i = 0U; i < list->count; ++i) {
        const elx_diagnostic_t *item = &list->items[i];
        if (source_name) {
            fprintf(stream, "%s:%zu:%zu: ELX-%d: %s\n", source_name, item->location.line, item->location.column, item->code, item->message ? item->message : "");
        } else {
            fprintf(stream, "%zu:%zu: ELX-%d: %s\n", item->location.line, item->location.column, item->code, item->message ? item->message : "");
        }
    }
}

elx_lexer_t *elx_lexer_create(const char *source, size_t length, elx_diagnostic_list_t *diagnostics) {
    elx_lexer_t *lexer;
    if (!source) return NULL;
    lexer = (elx_lexer_t *)calloc(1U, sizeof(*lexer));
    if (!lexer) return NULL;
    lexer->source = source;
    lexer->length = length;
    lexer->line = 1U;
    lexer->column = 1U;
    lexer->diagnostics = diagnostics;
    return lexer;
}

void elx_lexer_destroy(elx_lexer_t *lexer) {
    free(lexer);
}

const char *elx_token_type_name(elx_token_type_t type) {
    switch (type) {
        case ELX_TOKEN_EOF: return "eof";
        case ELX_TOKEN_IDENT: return "identifier";
        case ELX_TOKEN_INT: return "integer";
        case ELX_TOKEN_FLOAT: return "float";
        case ELX_TOKEN_STRING: return "string";
        case ELX_TOKEN_STRING_VIEW: return "string_view";
        case ELX_TOKEN_FN: return "fn";
        case ELX_TOKEN_STRUCT: return "struct";
        case ELX_TOKEN_ENUM: return "enum";
        case ELX_TOKEN_LET: return "let";
        case ELX_TOKEN_CONST: return "const";
        case ELX_TOKEN_VAR: return "var";
        case ELX_TOKEN_IF: return "if";
        case ELX_TOKEN_ELSE: return "else";
        case ELX_TOKEN_MATCH: return "match";
        case ELX_TOKEN_FOR: return "for";
        case ELX_TOKEN_WHILE: return "while";
        case ELX_TOKEN_LOOP: return "loop";
        case ELX_TOKEN_RETURN: return "return";
        case ELX_TOKEN_IMPORT: return "import";
        case ELX_TOKEN_MODULE: return "module";
        case ELX_TOKEN_USE: return "use";
        case ELX_TOKEN_PUB: return "pub";
        case ELX_TOKEN_PRIV: return "priv";
        case ELX_TOKEN_EXPORT: return "export";
        case ELX_TOKEN_NATIVE: return "native";
        case ELX_TOKEN_FFI: return "ffi";
        case ELX_TOKEN_SERVICE: return "service";
        case ELX_TOKEN_ASYNC: return "async";
        case ELX_TOKEN_AWAIT: return "await";
        case ELX_TOKEN_TRUE: return "true";
        case ELX_TOKEN_FALSE: return "false";
        case ELX_TOKEN_NIL: return "nil";
        case ELX_TOKEN_AS: return "as";
        case ELX_TOKEN_WHERE: return "where";
        case ELX_TOKEN_AND: return "and";
        case ELX_TOKEN_OR: return "or";
        case ELX_TOKEN_IN: return "in";
        case ELX_TOKEN_ON: return "on";
        case ELX_TOKEN_STATE: return "state";
        case ELX_TOKEN_WIDGET: return "widget";
        case ELX_TOKEN_PERMISSIONS: return "permissions";
        case ELX_TOKEN_CAPABILITY: return "capability";
        case ELX_TOKEN_TASK: return "task";
        case ELX_TOKEN_SELECT: return "select";
        case ELX_TOKEN_BREAK: return "break";
        case ELX_TOKEN_CONTINUE: return "continue";
        case ELX_TOKEN_IMPL: return "impl";
        case ELX_TOKEN_TYPE: return "type";
        case ELX_TOKEN_PLUS: return "+";
        case ELX_TOKEN_MINUS: return "-";
        case ELX_TOKEN_STAR: return "*";
        case ELX_TOKEN_SLASH: return "/";
        case ELX_TOKEN_PERCENT: return "%";
        case ELX_TOKEN_PLUS_ASSIGN: return "+=";
        case ELX_TOKEN_MINUS_ASSIGN: return "-=";
        case ELX_TOKEN_STAR_ASSIGN: return "*=";
        case ELX_TOKEN_SLASH_ASSIGN: return "/=";
        case ELX_TOKEN_PERCENT_ASSIGN: return "%=";
        case ELX_TOKEN_EQ: return "=";
        case ELX_TOKEN_EQ_EQ: return "==";
        case ELX_TOKEN_BANG: return "!";
        case ELX_TOKEN_BANG_EQ: return "!=";
        case ELX_TOKEN_LT: return "<";
        case ELX_TOKEN_LT_EQ: return "<=";
        case ELX_TOKEN_GT: return ">";
        case ELX_TOKEN_GT_EQ: return ">=";
        case ELX_TOKEN_AND_AND: return "&&";
        case ELX_TOKEN_OR_OR: return "||";
        case ELX_TOKEN_DOT: return ".";
        case ELX_TOKEN_DOT_DOT: return "..";
        case ELX_TOKEN_ARROW: return "->";
        case ELX_TOKEN_FAT_ARROW: return "=>";
        case ELX_TOKEN_ASSIGN: return "=";
        case ELX_TOKEN_COLON_ASSIGN: return ":=";
        case ELX_TOKEN_COLON: return ":";
        case ELX_TOKEN_DOUBLE_COLON: return "::";
        case ELX_TOKEN_COMMA: return ",";
        case ELX_TOKEN_SEMICOLON: return ";";
        case ELX_TOKEN_LPAREN: return "(";
        case ELX_TOKEN_RPAREN: return ")";
        case ELX_TOKEN_LBRACE: return "{";
        case ELX_TOKEN_RBRACE: return "}";
        case ELX_TOKEN_LBRACKET: return "[";
        case ELX_TOKEN_RBRACKET: return "]";
        case ELX_TOKEN_PIPE: return "|";
        case ELX_TOKEN_UNKNOWN: return "unknown";
    }
    return "unknown";
}

void elx_token_destroy(elx_token_t *token) {
    if (!token) return;
    free(token->string);
    token->string = NULL;
}

elx_type_t *elx_type_new(elx_type_kind_t kind) {
    elx_type_t *type = (elx_type_t *)calloc(1U, sizeof(*type));
    if (type) type->kind = kind;
    return type;
}

elx_type_t *elx_type_named(const char *name) {
    elx_type_t *type = elx_type_new(ELX_TYPE_NAMED);
    if (!type) return NULL;
    type->name = copy_string(name);
    if (!type->name) {
        free(type);
        return NULL;
    }
    return type;
}

elx_type_t *elx_type_ptr(elx_type_t *base) {
    elx_type_t *type = elx_type_new(ELX_TYPE_PTR);
    if (!type) return NULL;
    type->base = base;
    return type;
}

elx_type_t *elx_type_array(elx_type_t *base, size_t length) {
    elx_type_t *type = elx_type_new(ELX_TYPE_ARRAY);
    if (!type) return NULL;
    type->base = base;
    type->length = length;
    return type;
}

static elx_type_t *type_clone(const elx_type_t *source) {
    elx_type_t *type;
    size_t i;
    if (!source) return NULL;
    type = elx_type_new(source->kind);
    if (!type) return NULL;
    type->name = copy_string(source->name);
    type->base = type_clone(source->base);
    type->length = source->length;
    if (source->parameter_count > 0U) {
        type->parameters = (elx_type_t **)calloc(source->parameter_count, sizeof(*type->parameters));
        if (!type->parameters) {
            elx_type_destroy(type);
            return NULL;
        }
        type->parameter_count = source->parameter_count;
        for (i = 0U; i < source->parameter_count; ++i) type->parameters[i] = type_clone(source->parameters[i]);
    }
    type->return_type = type_clone(source->return_type);
    if ((source->name && !type->name) || (source->base && !type->base) || (source->return_type && !type->return_type)) {
        elx_type_destroy(type);
        return NULL;
    }
    return type;
}

elx_type_t *elx_type_fn(elx_type_t *return_type, elx_ast_t *parameters) {
    elx_type_t *type = elx_type_new(ELX_TYPE_FN);
    elx_ast_t *parameter;
    size_t count = 0U;
    size_t index = 0U;
    if (!type) return NULL;
    for (parameter = parameters; parameter; parameter = parameter->next) ++count;
    if (count > 0U) {
        type->parameters = (elx_type_t **)calloc(count, sizeof(*type->parameters));
        if (!type->parameters) {
            free(type);
            return NULL;
        }
    }
    for (parameter = parameters; parameter; parameter = parameter->next) {
        type->parameters[index++] = type_clone(parameter->type);
    }
    type->parameter_count = count;
    type->return_type = type_clone(return_type);
    if (!type->return_type) {
        elx_type_destroy(type);
        return NULL;
    }
    return type;
}

void elx_type_destroy(elx_type_t *type) {
    size_t i;
    if (!type) return;
    free(type->name);
    for (i = 0U; i < type->parameter_count; ++i) elx_type_destroy(type->parameters[i]);
    free(type->parameters);
    elx_type_destroy(type->base);
    elx_type_destroy(type->return_type);
    free(type);
}

elx_type_t *elx_type_clone(const elx_type_t *type) {
    return type_clone(type);
}

bool elx_type_equal(const elx_type_t *left, const elx_type_t *right) {
    elx_type_kind_t left_kind;
    elx_type_kind_t right_kind;
    size_t i;
    if (!left || !right) return left == right;
    left_kind = left->kind;
    right_kind = right->kind;
    if (left_kind == ELX_TYPE_NAMED && left->name) {
        if (strcmp(left->name, "Int") == 0) left_kind = ELX_TYPE_INT;
        else if (strcmp(left->name, "Float") == 0) left_kind = ELX_TYPE_FLOAT;
        else if (strcmp(left->name, "Bool") == 0) left_kind = ELX_TYPE_BOOL;
        else if (strcmp(left->name, "String") == 0) left_kind = ELX_TYPE_STRING;
        else if (strcmp(left->name, "Void") == 0) left_kind = ELX_TYPE_VOID;
    }
    if (right_kind == ELX_TYPE_NAMED && right->name) {
        if (strcmp(right->name, "Int") == 0) right_kind = ELX_TYPE_INT;
        else if (strcmp(right->name, "Float") == 0) right_kind = ELX_TYPE_FLOAT;
        else if (strcmp(right->name, "Bool") == 0) right_kind = ELX_TYPE_BOOL;
        else if (strcmp(right->name, "String") == 0) right_kind = ELX_TYPE_STRING;
        else if (strcmp(right->name, "Void") == 0) right_kind = ELX_TYPE_VOID;
    }
    if (left_kind != right_kind) return false;
    if (left_kind == ELX_TYPE_NAMED) {
        return left->name && right->name && strcmp(left->name, right->name) == 0;
    }
    if (!elx_type_equal(left->base, right->base)) return false;
    if (left->length != right->length) return false;
    if (left->parameter_count != right->parameter_count) return false;
    for (i = 0U; i < left->parameter_count; ++i) {
        if (!elx_type_equal(left->parameters[i], right->parameters[i])) return false;
    }
    return elx_type_equal(left->return_type, right->return_type);
}

elx_ast_t *elx_ast_new(elx_ast_kind_t kind, elx_location_t location) {
    elx_ast_t *node = (elx_ast_t *)calloc(1U, sizeof(*node));
    if (!node) return NULL;
    node->kind = kind;
    node->location = location;
    node->end = location;
    return node;
}

elx_ast_t *elx_ast_append(elx_ast_t *list, elx_ast_t *item) {
    elx_ast_t *cursor;
    if (!item) return list;
    if (!list) return item;
    cursor = list;
    while (cursor->next) cursor = cursor->next;
    cursor->next = item;
    return list;
}

size_t elx_ast_count(const elx_ast_t *list) {
    size_t count = 0U;
    while (list) {
        ++count;
        list = list->next;
    }
    return count;
}

static void print_indent(FILE *stream, size_t depth) {
    size_t i;
    for (i = 0U; i < depth; ++i) fputs("  ", stream);
}

static void ast_print_inner(const elx_ast_t *node, FILE *stream, size_t depth);

static void print_child(const char *label, const elx_ast_t *child, FILE *stream, size_t depth) {
    if (!child) return;
    print_indent(stream, depth);
    fprintf(stream, "%s:\n", label);
    ast_print_inner(child, stream, depth + 1U);
}

static void ast_print_inner(const elx_ast_t *node, FILE *stream, size_t depth) {
    if (!node) return;
    print_indent(stream, depth);
    switch (node->kind) {
        case ELX_AST_PROGRAM: fputs("program", stream); break;
        case ELX_AST_FUNCTION: fputs("function", stream); break;
        case ELX_AST_STRUCT: fputs("struct", stream); break;
        case ELX_AST_ENUM: fputs("enum", stream); break;
        case ELX_AST_ENUM_MEMBER: fputs("enum_member", stream); break;
        case ELX_AST_CONST: fputs("const", stream); break;
        case ELX_AST_VAR: fputs(node->mutable_value ? "var" : "let", stream); break;
        case ELX_AST_IMPORT: fputs("import", stream); break;
        case ELX_AST_MODULE: fputs("module", stream); break;
        case ELX_AST_EXPORT: fputs("export", stream); break;
        case ELX_AST_PERMISSIONS: fputs("permissions", stream); break;
        case ELX_AST_CAPABILITY: fputs("capability", stream); break;
        case ELX_AST_NATIVE: fputs("native", stream); break;
        case ELX_AST_FFI: fputs("ffi", stream); break;
        case ELX_AST_SERVICE: fputs("service", stream); break;
        case ELX_AST_TASK: fputs("task", stream); break;
        case ELX_AST_STATE: fputs("state", stream); break;
        case ELX_AST_WIDGET: fputs("widget", stream); break;
        case ELX_AST_BLOCK: fputs("block", stream); break;
        case ELX_AST_IF: fputs("if", stream); break;
        case ELX_AST_MATCH: fputs("match", stream); break;
        case ELX_AST_MATCH_ARM: fputs("match_arm", stream); break;
        case ELX_AST_FOR: fputs("for", stream); break;
        case ELX_AST_WHILE: fputs("while", stream); break;
        case ELX_AST_LOOP: fputs("loop", stream); break;
        case ELX_AST_RETURN: fputs("return", stream); break;
        case ELX_AST_BREAK: fputs("break", stream); break;
        case ELX_AST_CONTINUE: fputs("continue", stream); break;
        case ELX_AST_EXPR_STMT: fputs("expression", stream); break;
        case ELX_AST_INT: fprintf(stream, "int(%llu)", (unsigned long long)node->integer); break;
        case ELX_AST_FLOAT: fprintf(stream, "float(%g)", node->real); break;
        case ELX_AST_STRING: fprintf(stream, "string(\"%s\")", node->text ? node->text : ""); break;
        case ELX_AST_BOOL: fputs(node->boolean ? "true" : "false", stream); break;
        case ELX_AST_NIL: fputs("nil", stream); break;
        case ELX_AST_IDENT: fprintf(stream, "ident(%s)", node->name ? node->name : ""); break;
        case ELX_AST_BINARY: fprintf(stream, "binary(%s)", elx_token_type_name(node->operator)); break;
        case ELX_AST_UNARY: fprintf(stream, "unary(%s)", elx_token_type_name(node->operator)); break;
        case ELX_AST_ASSIGN: fputs("assign", stream); break;
        case ELX_AST_CALL: fputs("call", stream); break;
        case ELX_AST_INDEX: fputs("index", stream); break;
        case ELX_AST_MEMBER: fprintf(stream, "member(%s)", node->name ? node->name : ""); break;
        case ELX_AST_ARRAY: fputs("array", stream); break;
        case ELX_AST_STRUCT_LITERAL: fprintf(stream, "struct_literal(%s)", node->name ? node->name : ""); break;
        case ELX_AST_LAMBDA: fputs("lambda", stream); break;
        case ELX_AST_UI_PROPERTY: fprintf(stream, "property(%s)", node->name ? node->name : ""); break;
        case ELX_AST_UI_EVENT: fprintf(stream, "event(%s)", node->name ? node->name : ""); break;
    }
    if (node->name && node->kind != ELX_AST_IDENT && node->kind != ELX_AST_MEMBER && node->kind != ELX_AST_UI_PROPERTY && node->kind != ELX_AST_UI_EVENT && node->kind != ELX_AST_STRUCT_LITERAL) fprintf(stream, " %s", node->name);
    fputc('\n', stream);
    print_child("condition", node->condition, stream, depth);
    print_child("body", node->body, stream, depth);
    print_child("else", node->else_body, stream, depth);
    print_child("init", node->init, stream, depth);
    print_child("step", node->step, stream, depth);
    print_child("target", node->target, stream, depth);
    print_child("value", node->value, stream, depth);
    print_child("left", node->left, stream, depth);
    print_child("right", node->right, stream, depth);
    print_child("third", node->third, stream, depth);
    print_child("callee", node->callee, stream, depth);
    print_child("args", node->args, stream, depth);
    print_child("object", node->object, stream, depth);
    print_child("items", node->items, stream, depth);
    print_child("properties", node->properties, stream, depth);
    print_child("children", node->children, stream, depth);
}

void elx_ast_print(const elx_ast_t *node, FILE *stream) {
    if (!node || !stream) return;
    while (node) {
        ast_print_inner(node, stream, 0U);
        node = node->next;
    }
}

void elx_ast_destroy(elx_ast_t *node) {
    if (!node) return;
    free(node->name);
    free(node->text);
    elx_type_destroy(node->type);
    elx_ast_destroy(node->next);
    elx_ast_destroy(node->left);
    elx_ast_destroy(node->right);
    elx_ast_destroy(node->third);
    elx_ast_destroy(node->body);
    elx_ast_destroy(node->else_body);
    elx_ast_destroy(node->condition);
    elx_ast_destroy(node->init);
    elx_ast_destroy(node->step);
    elx_ast_destroy(node->target);
    elx_ast_destroy(node->value);
    elx_ast_destroy(node->callee);
    elx_ast_destroy(node->args);
    elx_ast_destroy(node->object);
    elx_ast_destroy(node->items);
    elx_ast_destroy(node->properties);
    elx_ast_destroy(node->children);
    free(node);
}
