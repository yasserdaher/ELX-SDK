#ifndef ELX_H
#define ELX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ELX_TOKEN_EOF = 0,
    ELX_TOKEN_IDENT,
    ELX_TOKEN_INT,
    ELX_TOKEN_FLOAT,
    ELX_TOKEN_STRING,
    ELX_TOKEN_STRING_VIEW,
    ELX_TOKEN_FN,
    ELX_TOKEN_STRUCT,
    ELX_TOKEN_ENUM,
    ELX_TOKEN_LET,
    ELX_TOKEN_CONST,
    ELX_TOKEN_VAR,
    ELX_TOKEN_IF,
    ELX_TOKEN_ELSE,
    ELX_TOKEN_MATCH,
    ELX_TOKEN_FOR,
    ELX_TOKEN_WHILE,
    ELX_TOKEN_LOOP,
    ELX_TOKEN_RETURN,
    ELX_TOKEN_IMPORT,
    ELX_TOKEN_MODULE,
    ELX_TOKEN_USE,
    ELX_TOKEN_PUB,
    ELX_TOKEN_PRIV,
    ELX_TOKEN_EXPORT,
    ELX_TOKEN_NATIVE,
    ELX_TOKEN_FFI,
    ELX_TOKEN_SERVICE,
    ELX_TOKEN_ASYNC,
    ELX_TOKEN_AWAIT,
    ELX_TOKEN_TRUE,
    ELX_TOKEN_FALSE,
    ELX_TOKEN_NIL,
    ELX_TOKEN_AS,
    ELX_TOKEN_WHERE,
    ELX_TOKEN_AND,
    ELX_TOKEN_OR,
    ELX_TOKEN_IN,
    ELX_TOKEN_ON,
    ELX_TOKEN_STATE,
    ELX_TOKEN_WIDGET,
    ELX_TOKEN_PERMISSIONS,
    ELX_TOKEN_CAPABILITY,
    ELX_TOKEN_TASK,
    ELX_TOKEN_SELECT,
    ELX_TOKEN_BREAK,
    ELX_TOKEN_CONTINUE,
    ELX_TOKEN_IMPL,
    ELX_TOKEN_TYPE,
    ELX_TOKEN_PLUS,
    ELX_TOKEN_MINUS,
    ELX_TOKEN_STAR,
    ELX_TOKEN_SLASH,
    ELX_TOKEN_PERCENT,
    ELX_TOKEN_PLUS_ASSIGN,
    ELX_TOKEN_MINUS_ASSIGN,
    ELX_TOKEN_STAR_ASSIGN,
    ELX_TOKEN_SLASH_ASSIGN,
    ELX_TOKEN_PERCENT_ASSIGN,
    ELX_TOKEN_EQ,
    ELX_TOKEN_EQ_EQ,
    ELX_TOKEN_BANG,
    ELX_TOKEN_BANG_EQ,
    ELX_TOKEN_LT,
    ELX_TOKEN_LT_EQ,
    ELX_TOKEN_GT,
    ELX_TOKEN_GT_EQ,
    ELX_TOKEN_AND_AND,
    ELX_TOKEN_OR_OR,
    ELX_TOKEN_PIPE,
    ELX_TOKEN_DOT,
    ELX_TOKEN_DOT_DOT,
    ELX_TOKEN_ARROW,
    ELX_TOKEN_FAT_ARROW,
    ELX_TOKEN_ASSIGN,
    ELX_TOKEN_COLON_ASSIGN,
    ELX_TOKEN_COLON,
    ELX_TOKEN_DOUBLE_COLON,
    ELX_TOKEN_COMMA,
    ELX_TOKEN_SEMICOLON,
    ELX_TOKEN_LPAREN,
    ELX_TOKEN_RPAREN,
    ELX_TOKEN_LBRACE,
    ELX_TOKEN_RBRACE,
    ELX_TOKEN_LBRACKET,
    ELX_TOKEN_RBRACKET,
    ELX_TOKEN_UNKNOWN
} elx_token_type_t;

typedef struct {
    size_t line;
    size_t column;
    size_t offset;
} elx_location_t;

typedef struct {
    int code;
    const char *message;
    elx_location_t location;
} elx_diagnostic_t;

typedef struct {
    elx_diagnostic_t *items;
    size_t count;
    size_t capacity;
    bool has_errors;
} elx_diagnostic_list_t;

typedef struct {
    const char *source;
    size_t length;
    size_t position;
    size_t line;
    size_t column;
    elx_diagnostic_list_t *diagnostics;
} elx_lexer_t;

typedef struct {
    elx_token_type_t type;
    const char *text;
    size_t length;
    uint64_t integer;
    double real;
    char *string;
    elx_location_t location;
    elx_location_t end;
} elx_token_t;

typedef enum {
    ELX_TYPE_INVALID = 0,
    ELX_TYPE_VOID,
    ELX_TYPE_BOOL,
    ELX_TYPE_INT,
    ELX_TYPE_FLOAT,
    ELX_TYPE_STRING,
    ELX_TYPE_STRING_VIEW,
    ELX_TYPE_PTR,
    ELX_TYPE_ARRAY,
    ELX_TYPE_FN,
    ELX_TYPE_NAMED,
    ELX_TYPE_WIDGET
} elx_type_kind_t;

typedef struct elx_type {
    elx_type_kind_t kind;
    char *name;
    struct elx_type *base;
    size_t length;
    struct elx_type **parameters;
    size_t parameter_count;
    struct elx_type *return_type;
} elx_type_t;

typedef enum {
    ELX_AST_PROGRAM = 0,
    ELX_AST_FUNCTION,
    ELX_AST_STRUCT,
    ELX_AST_ENUM,
    ELX_AST_ENUM_MEMBER,
    ELX_AST_CONST,
    ELX_AST_VAR,
    ELX_AST_IMPORT,
    ELX_AST_MODULE,
    ELX_AST_EXPORT,
    ELX_AST_PERMISSIONS,
    ELX_AST_CAPABILITY,
    ELX_AST_NATIVE,
    ELX_AST_FFI,
    ELX_AST_SERVICE,
    ELX_AST_TASK,
    ELX_AST_STATE,
    ELX_AST_WIDGET,
    ELX_AST_BLOCK,
    ELX_AST_IF,
    ELX_AST_MATCH,
    ELX_AST_MATCH_ARM,
    ELX_AST_FOR,
    ELX_AST_WHILE,
    ELX_AST_LOOP,
    ELX_AST_RETURN,
    ELX_AST_BREAK,
    ELX_AST_CONTINUE,
    ELX_AST_EXPR_STMT,
    ELX_AST_INT,
    ELX_AST_FLOAT,
    ELX_AST_STRING,
    ELX_AST_BOOL,
    ELX_AST_NIL,
    ELX_AST_IDENT,
    ELX_AST_BINARY,
    ELX_AST_UNARY,
    ELX_AST_ASSIGN,
    ELX_AST_CALL,
    ELX_AST_INDEX,
    ELX_AST_MEMBER,
    ELX_AST_ARRAY,
    ELX_AST_STRUCT_LITERAL,
    ELX_AST_LAMBDA,
    ELX_AST_UI_PROPERTY,
    ELX_AST_UI_EVENT
} elx_ast_kind_t;

typedef struct elx_ast {
    elx_ast_kind_t kind;
    elx_location_t location;
    elx_location_t end;
    elx_type_t *type;
    struct elx_ast *next;
    char *name;
    char *text;
    uint64_t integer;
    double real;
    bool boolean;
    bool mutable_value;
    bool public_value;
    bool async_value;
    elx_token_type_t operator;
    struct elx_ast *left;
    struct elx_ast *right;
    struct elx_ast *third;
    struct elx_ast *body;
    struct elx_ast *else_body;
    struct elx_ast *condition;
    struct elx_ast *init;
    struct elx_ast *step;
    struct elx_ast *target;
    struct elx_ast *value;
    struct elx_ast *callee;
    char *func_name;
    struct elx_ast *args;
    struct elx_ast *object;
    struct elx_ast *items;
    struct elx_ast *properties;
    char *layout_hint;
    bool reactive;
    bool used;
    bool tail_call;
    bool stack_allocated;
    uint32_t vm_slot;
    struct elx_ast *children;
    char *import_path;
    char *alias;
    char *module_name;
    struct elx_ast *resolved_module;
    bool is_public;
} elx_ast_t;

typedef struct {
    char *name;
    elx_type_t *type;
    bool mutable_value;
    bool public_value;
    bool defined;
    size_t scope_depth;
} elx_symbol_t;

typedef struct elx_scope {
    elx_symbol_t *symbols;
    size_t count;
    size_t capacity;
    struct elx_scope *parent;
} elx_scope_t;

typedef struct {
    elx_scope_t *global;
    elx_scope_t *current;
    size_t scope_depth;
    elx_diagnostic_list_t *diagnostics;
} elx_semantic_context_t;

typedef struct {
    size_t widget_count;
    size_t state_count;
    size_t function_count;
    size_t capability_count;
    size_t folded_constants;
    size_t combined_instructions;
    size_t eliminated_dead_code;
    size_t tail_calls;
    size_t stack_allocations;
    size_t removed_unused_widgets;
    size_t reactive_edges;
    size_t update_order_count;
    char **update_order;
} elx_optimization_report_t;

typedef struct {
    bool dump_tokens;
    bool dump_ast;
    bool optimize;
    char *module_path;
    char *stdlib_path;
    char *output_path;
} elx_compile_options_t;

typedef struct elx_compile_module_info {
    char *name;
    char *path;
    char **imports;
    size_t import_count;
    char **exports;
    size_t export_count;
    uint32_t version;
    bool has_errors;
} elx_compile_module_info_t;

typedef struct elx_compile_module {
    char *name;
    char *path;
    elx_ast_t *ast;
    elx_scope_t *global_scope;
    char **public_symbols;
    size_t public_count;
    struct elx_compile_module *next;
    bool loaded;
    bool loading;
} elx_compile_module_t;

void elx_diagnostics_init(elx_diagnostic_list_t *list);
void elx_diagnostics_destroy(elx_diagnostic_list_t *list);
void elx_diagnostic_add(elx_diagnostic_list_t *list, int code, const char *message, elx_location_t location);
void elx_diagnostics_print(const elx_diagnostic_list_t *list, const char *source_name, FILE *stream);

elx_lexer_t *elx_lexer_create(const char *source, size_t length, elx_diagnostic_list_t *diagnostics);
void elx_lexer_destroy(elx_lexer_t *lexer);
elx_token_t elx_lexer_next(elx_lexer_t *lexer);
void elx_token_destroy(elx_token_t *token);
const char *elx_token_type_name(elx_token_type_t type);

elx_type_t *elx_type_new(elx_type_kind_t kind);
elx_type_t *elx_type_named(const char *name);
elx_type_t *elx_type_ptr(elx_type_t *base);
elx_type_t *elx_type_array(elx_type_t *base, size_t length);
elx_type_t *elx_type_fn(elx_type_t *return_type, elx_ast_t *parameters);
void elx_type_destroy(elx_type_t *type);
elx_type_t *elx_type_clone(const elx_type_t *type);
bool elx_type_equal(const elx_type_t *left, const elx_type_t *right);

elx_ast_t *elx_ast_new(elx_ast_kind_t kind, elx_location_t location);
void elx_ast_destroy(elx_ast_t *node);
elx_ast_t *elx_ast_append(elx_ast_t *list, elx_ast_t *item);
size_t elx_ast_count(const elx_ast_t *list);
void elx_ast_print(const elx_ast_t *node, FILE *stream);

elx_ast_t *elx_parse(const char *source, size_t length, elx_diagnostic_list_t *diagnostics);
bool elx_semantic_analyze(elx_ast_t *program, elx_diagnostic_list_t *diagnostics);
void elx_optimize(elx_ast_t *program, elx_optimization_report_t *report);
bool elx_write_elbc(const elx_ast_t *program, const char *output_path, elx_diagnostic_list_t *diagnostics);
bool elx_compile_source(const char *source, size_t length, const char *output_path, const elx_compile_options_t *options, elx_optimization_report_t *report, elx_diagnostic_list_t *diagnostics);
bool elx_compile_file(const char *input_path, const char *output_path, const elx_compile_options_t *options, elx_optimization_report_t *report, elx_diagnostic_list_t *diagnostics);
const char *elx_version(void);

elx_compile_module_t *elx_module_load_source(const char *path, elx_diagnostic_list_t *diagnostics);
void elx_module_free(elx_compile_module_t *module);
elx_ast_t *elx_module_resolve(const char *name, const char *search_path, elx_diagnostic_list_t *diagnostics);
bool elx_compile_module(const char *source, size_t length, const char *module_name, const char *output_path, const elx_compile_options_t *options, elx_optimization_report_t *report, elx_diagnostic_list_t *diagnostics);

#ifdef __cplusplus
}
#endif

#endif
