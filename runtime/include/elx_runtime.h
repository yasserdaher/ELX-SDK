#ifndef ELX_RUNTIME_H
#define ELX_RUNTIME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELX_RUNTIME_VERSION "0.1.0"
#define ELX_VM_MAX_REGISTERS 65536
#define ELX_VM_STACK_SIZE 1024
#define ELX_VM_WIDGET_STACK_SIZE 256
#define ELX_STRING_POOL_INIT 128
#define ELX_ARENA_BLOCK_SIZE 65536U

typedef enum {
    ELX_OP_NOP = 0x00,
    ELX_OP_LOAD_CONST = 0x01,
    ELX_OP_LOAD_INT = 0x02,
    ELX_OP_LOAD_FLOAT = 0x03,
    ELX_OP_LOAD_BOOL = 0x04,
    ELX_OP_LOAD_NIL = 0x05,
    ELX_OP_STORE_LOCAL = 0x06,
    ELX_OP_LOAD_LOCAL = 0x07,
    ELX_OP_CALL = 0x09,
    ELX_OP_RETURN = 0x0A,
    ELX_OP_JUMP = 0x1A,
    ELX_OP_JUMP_IF_FALSE = 0x1B,
    ELX_OP_JUMP_IF_TRUE = 0x1C,
    ELX_OP_LOOP_START = 0x1D,
    ELX_OP_LOOP_END = 0x1E,
    ELX_OP_BINARY = 0x0B,
    ELX_OP_UNARY = 0x0C,
    ELX_OP_MEMBER = 0x0D,
    ELX_OP_INDEX = 0x0E,
    ELX_OP_STACK_ALLOC = 0x0F,
    ELX_OP_BLOCK_END = 0x10,
    ELX_OP_WIDGET_CREATE = 0x14,
    ELX_OP_WIDGET_PROPERTY = 0x15,
    ELX_OP_STATE_SUBSCRIBE = 0x16,
    ELX_OP_STRING = 0x21,
    ELX_OP_CALL_TAIL = 0x20,
    ELX_OP_NATIVE = 0x22,
    ELX_OP_RETURN_TAIL_FLAG = 0x01
} elx_opcode_t;

typedef enum {
    ELX_BIN_ADD = 1,
    ELX_BIN_SUB = 2,
    ELX_BIN_MUL = 3,
    ELX_BIN_DIV = 4,
    ELX_BIN_MOD = 5,
    ELX_BIN_LT = 6,
    ELX_BIN_LE = 7,
    ELX_BIN_GT = 8,
    ELX_BIN_GE = 9,
    ELX_BIN_EQ = 10,
    ELX_BIN_NE = 11,
    ELX_BIN_AND = 12,
    ELX_BIN_OR = 13
} elx_binary_op_t;

typedef enum {
    ELX_UNARY_NOT = 1,
    ELX_UNARY_NEG = 2
} elx_unary_op_t;

typedef enum {
    ELX_VAL_NIL,
    ELX_VAL_BOOL,
    ELX_VAL_INT,
    ELX_VAL_FLOAT,
    ELX_VAL_STRING,
    ELX_VAL_WIDGET
} elx_value_type_t;

typedef struct elx_value {
    elx_value_type_t type;
    union {
        bool boolean;
        int64_t integer;
        double real;
        const char *string;
        void *widget;
    };
} elx_value_t;

typedef struct {
    elx_value_t *items;
    size_t count;
    size_t capacity;
} elx_stack_t;

typedef struct {
    elx_value_t *registers;
    size_t register_count;
} elx_vm_t;

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t cursor;
} elx_reader_t;

typedef struct {
    char **strings;
    size_t count;
    size_t capacity;
} elx_string_pool_t;

typedef struct elx_function {
    char *name;
    uint32_t arg_count;
    uint32_t start_slot;
    uint32_t code_size;
    size_t code_offset;
    bool is_exported;
    struct elx_function *next;
} elx_function_t;

typedef struct {
    char *module_name;
    char *import_path;
    char *alias;
} elx_import_t;

typedef struct {
    char *name;
    uint32_t function_index;
} elx_export_t;

typedef struct {
    const char *name;
    uint32_t arg_count;
    elx_value_t (*fn)(elx_value_t *args, uint32_t count);
} elx_native_fn_t;

typedef struct {
    elx_native_fn_t *items;
    size_t count;
    size_t capacity;
} elx_native_table_t;

typedef struct {
    const uint8_t *data;
    size_t data_size;
    char *name;
    uint16_t version;
    uint32_t flags;
    uint64_t timestamp;
    uint32_t function_count;
    uint32_t widget_count;
    uint32_t string_count;
    uint32_t register_count;
    uint32_t import_count;
    uint32_t export_count;
    uint32_t checksum;
    elx_function_t *functions;
    elx_import_t *imports;
    elx_export_t *exports;
    elx_string_pool_t string_pool;
    size_t code_start;
} elx_module_t;

typedef struct {
    uint8_t *base;
    uint8_t *data;
    size_t used;
    size_t capacity;
} elx_arena_t;

typedef struct {
    elx_vm_t vm;
    elx_stack_t call_stack;
    elx_module_t *module;
    elx_arena_t *arena;
    elx_value_t last_result;
    elx_native_table_t natives;
    bool running;
    int exit_code;
    void *widget_stack[ELX_VM_WIDGET_STACK_SIZE];
    size_t widget_stack_count;
} elx_runtime_t;

elx_value_t elx_value_nil(void);
elx_value_t elx_value_bool(bool value);
elx_value_t elx_value_int(int64_t value);
elx_value_t elx_value_float(double value);
elx_value_t elx_value_string(const char *value);
elx_value_t elx_value_copy(elx_value_t value);
void elx_value_destroy(elx_value_t value);
const char *elx_value_to_string(const elx_value_t *value);

elx_reader_t *elx_reader_create(const uint8_t *data, size_t size);
void elx_reader_destroy(elx_reader_t *reader);
bool elx_reader_read_u8(elx_reader_t *reader, uint8_t *out);
bool elx_reader_read_u16(elx_reader_t *reader, uint16_t *out);
bool elx_reader_read_u32(elx_reader_t *reader, uint32_t *out);
bool elx_reader_read_u64(elx_reader_t *reader, uint64_t *out);
bool elx_reader_read_bytes(elx_reader_t *reader, void *out, size_t count);
bool elx_reader_read_string(elx_reader_t *reader, char *buffer, size_t max_len);

bool elx_module_load(const char *path, elx_module_t *module);
const char *elx_module_load_error(void);
void elx_module_destroy(elx_module_t *module);
elx_function_t *elx_module_find_function(const elx_module_t *module, const char *name);
const elx_export_t *elx_module_find_export(const elx_module_t *module, const char *name);
bool elx_module_link_import(const elx_module_t *module, const char *import_name,
                            elx_module_t *target, const char *target_name);

typedef struct {
    elx_module_t **modules;
    size_t count;
    size_t capacity;
} elx_module_registry_t;

typedef void (*elx_link_error_cb)(int code, const char *message, void *user_data);

bool elx_registry_load(elx_module_registry_t *registry, const char *path);
void elx_registry_destroy(elx_module_registry_t *registry);
elx_function_t *elx_registry_resolve(const elx_module_registry_t *registry,
                                     const char *module_name, const char *symbol);
bool elx_registry_link_all(elx_module_registry_t *registry,
                           elx_link_error_cb error_cb, void *user_data);

bool elx_arena_create(elx_arena_t *arena, size_t initial_size);
void elx_arena_destroy(elx_arena_t *arena);
void *elx_arena_alloc(elx_arena_t *arena, size_t size);
void elx_arena_reset(elx_arena_t *arena);

void elx_stack_init(elx_stack_t *stack);
void elx_stack_destroy(elx_stack_t *stack);
bool elx_stack_push(elx_stack_t *stack, elx_value_t value);
elx_value_t elx_stack_pop(elx_stack_t *stack);
elx_value_t elx_stack_peek(const elx_stack_t *stack, int32_t depth);

void elx_vm_init(elx_vm_t *vm, size_t register_count);
void elx_vm_destroy(elx_vm_t *vm);
elx_value_t *elx_vm_register(elx_vm_t *vm, uint32_t index);

bool elx_runtime_create(elx_runtime_t *runtime, const char *elbc_path);
void elx_runtime_destroy(elx_runtime_t *runtime);
int elx_runtime_run(elx_runtime_t *runtime, const char *entry_point);
bool elx_runtime_call_function(elx_runtime_t *runtime, const char *name);

void elx_native_register(elx_runtime_t *runtime, const char *name, uint32_t arg_count,
                         elx_value_t (*fn)(elx_value_t *args, uint32_t count));
const elx_native_fn_t *elx_native_lookup(const elx_runtime_t *runtime, const char *name);

void elx_stdlib_register(elx_runtime_t *runtime);

int elx_binary_op(elx_binary_op_t op, elx_value_t left, elx_value_t right, elx_value_t *result);
int elx_unary_op(elx_unary_op_t op, elx_value_t operand, elx_value_t *result);

const char *elx_runtime_version(void);
elx_value_t native_print(elx_value_t *args, uint32_t count);
elx_value_t native_clock(elx_value_t *args, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif
