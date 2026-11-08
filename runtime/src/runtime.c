/*
 * ELVI OS — VOLT 1.26 License Agreement (ELECTRON VISION)
 * See LICENSE at the project root for full terms.
 */
#include "elx_runtime.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

const char *elx_runtime_version(void) {
    return ELX_RUNTIME_VERSION;
}

static uint32_t read_u32(const uint8_t *code, size_t *pc) {
    uint32_t val = code[*pc] | (code[*pc + 1] << 8) | (code[*pc + 2] << 16) | (code[*pc + 3] << 24);
    *pc += 4U;
    return val;
}

static uint64_t read_u64(const uint8_t *code, size_t *pc) {
    uint64_t val = 0;
    size_t i;
    for (i = 0U; i < 8U; ++i) val |= (uint64_t)code[*pc + i] << (8U * i);
    *pc += 8U;
    return val;
}

static bool execute_bytecode(elx_runtime_t *runtime, const uint8_t *code, size_t code_size,
                             elx_value_t *stack, size_t *sp, elx_value_t *return_value);

static bool execute_call(elx_runtime_t *runtime, const uint8_t *code, size_t code_size,
                         const elx_function_t *func, elx_value_t *stack, size_t *sp,
                         elx_value_t *return_value) {
    elx_value_t *saved_regs = (elx_value_t *)malloc(runtime->vm.register_count * sizeof(elx_value_t));
    size_t arg_count_start = *sp - (size_t)func->arg_count;
    size_t i, reg_count = runtime->vm.register_count;
    if (!saved_regs) return false;
    if (!code || code_size == 0U) {
        free(saved_regs);
        return false;
    }
    memcpy(saved_regs, runtime->vm.registers, reg_count * sizeof(elx_value_t));
    memset(runtime->vm.registers, 0, reg_count * sizeof(elx_value_t));
    for (i = 0U; i < func->arg_count && i < reg_count; ++i) {
        runtime->vm.registers[func->start_slot + i] = stack[arg_count_start + i];
    }
    *sp = arg_count_start;
    {
        bool ok = execute_bytecode(runtime, code, code_size, stack, sp, return_value);
        *sp = arg_count_start;
        memcpy(runtime->vm.registers, saved_regs, reg_count * sizeof(elx_value_t));
        free(saved_regs);
        return ok;
    }
}

static bool execute_bytecode(elx_runtime_t *runtime, const uint8_t *code, size_t code_size,
                             elx_value_t *stack, size_t *sp, elx_value_t *return_value) {
    size_t pc = 0;
    if (!code || code_size == 0U) return false;
    if (!stack || !sp) return false;
    while (pc < code_size && runtime->running) {
        uint8_t opcode = code[pc++];
        uint32_t index;
        int64_t ival;
        uint64_t raw;
        double dval;
        bool bval;
        elx_value_t v;
        if (*sp >= ELX_VM_STACK_SIZE - 16U) {
            fprintf(stderr, "elx_runtime: stack overflow\n");
            return false;
        }
        switch (opcode) {
            case ELX_OP_NOP:
                break;
            case ELX_OP_LOAD_CONST: {
                if (pc + 4U > code_size) return false;
                index = read_u32(code, &pc);
                if (index == 0U) {
                    v.type = ELX_VAL_NIL;
                    v.integer = 0;
                    stack[(*sp)++] = v;
                    break;
                }
                if (index > runtime->module->string_count) {
                    fprintf(stderr, "elx_runtime: invalid string constant index %u\n", index);
                    return false;
                }
                v.type = ELX_VAL_STRING;
                v.string = runtime->module->string_pool.strings[index - 1U];
                stack[(*sp)++] = v;
                break;
            }
            case ELX_OP_LOAD_INT: {
                if (pc + 8U > code_size) return false;
                raw = read_u64(code, &pc);
                ival = (int64_t)raw;
                v.type = ELX_VAL_INT;
                v.integer = ival;
                stack[(*sp)++] = v;
                break;
            }
            case ELX_OP_LOAD_FLOAT: {
                if (pc + 8U > code_size) return false;
                raw = read_u64(code, &pc);
                memcpy(&dval, &raw, sizeof(dval));
                v.type = ELX_VAL_FLOAT;
                v.real = dval;
                stack[(*sp)++] = v;
                break;
            }
            case ELX_OP_LOAD_BOOL: {
                if (pc + 1U > code_size) return false;
                bval = code[pc++] != 0U;
                v.type = ELX_VAL_BOOL;
                v.boolean = bval;
                stack[(*sp)++] = v;
                break;
            }
            case ELX_OP_LOAD_NIL: {
                v.type = ELX_VAL_NIL;
                v.integer = 0;
                stack[(*sp)++] = v;
                break;
            }
            case ELX_OP_STORE_LOCAL: {
                if (pc + 4U > code_size) return false;
                index = read_u32(code, &pc);
                if (*sp == 0U) return false;
                elx_value_t *reg = elx_vm_register(&runtime->vm, index);
                if (reg) *reg = stack[--(*sp)];
                break;
            }
            case ELX_OP_LOAD_LOCAL: {
                if (pc + 4U > code_size) return false;
                index = read_u32(code, &pc);
                elx_value_t *reg = elx_vm_register(&runtime->vm, index);
                if (reg) {
                    stack[(*sp)++] = *reg;
                } else {
                    v.type = ELX_VAL_NIL;
                    v.integer = 0;
                    stack[(*sp)++] = v;
                }
                break;
            }
            case ELX_OP_CALL: {
                if (pc + 6U > code_size) return false;
                {
                    uint32_t func_slot = read_u32(code, &pc);
                    uint16_t arg_count = code[pc] | (code[pc + 1] << 8);
                    pc += 2U;
                    elx_function_t *callee_fn = NULL;
                    const char *func_name = NULL;
                    if (func_slot > 0U && func_slot <= runtime->module->string_count) {
                        func_name = runtime->module->string_pool.strings[func_slot - 1U];
                    }
                    if (func_name) {
                        callee_fn = elx_module_find_function(runtime->module, func_name);
                    }
                    if (!callee_fn) {
                        const elx_native_fn_t *native = elx_native_lookup(runtime, func_name);
                        if (native && native->fn) {
                            elx_value_t result;
                            result.type = ELX_VAL_NIL;
                            result.integer = 0;
                            if (*sp < arg_count) {
                                fprintf(stderr, "elx_runtime: not enough arguments for native '%s'\n", func_name);
                                return false;
                            }
                            result = native->fn(stack + (*sp - arg_count), arg_count);
                            *sp -= (size_t)arg_count;
                            stack[(*sp)++] = result;
                            break;
                        }
                        fprintf(stderr, "elx_runtime: function '%s' not found\n", func_name ? func_name : "(null)");
                        return false;
                    }
                    if (*sp < arg_count) {
                        fprintf(stderr, "elx_runtime: not enough arguments for call to '%s'\n", func_name);
                        return false;
                    }
                    {
                        const uint8_t *call_code = runtime->module->data + callee_fn->code_offset;
                        size_t call_code_size = (size_t)callee_fn->code_size;
                        elx_value_t call_return;
                        call_return.type = ELX_VAL_NIL;
                        call_return.integer = 0;
                    if (!execute_call(runtime, call_code, call_code_size,
                                          callee_fn, stack, sp, &call_return)) {
                            return false;
                        }
                        if (return_value) {
                            stack[(*sp)++] = call_return;
                        }
                    }
                    break;
                }
            }
            case ELX_OP_CALL_TAIL: {
                if (pc + 6U > code_size) return false;
                {
                    uint32_t func_slot = read_u32(code, &pc);
                    uint16_t arg_count = code[pc] | (code[pc + 1] << 8);
                    pc += 2U;
                    elx_function_t *callee_fn = NULL;
                    const char *func_name = NULL;
                    if (func_slot > 0U && func_slot <= runtime->module->string_count) {
                        func_name = runtime->module->string_pool.strings[func_slot - 1U];
                    }
                    if (func_name) {
                        callee_fn = elx_module_find_function(runtime->module, func_name);
                    }
                    if (!callee_fn) {
                        fprintf(stderr, "elx_runtime: function '%s' not found\n", func_name ? func_name : "(null)");
                        return false;
                    }
                    if (*sp < arg_count) {
                        fprintf(stderr, "elx_runtime: not enough arguments for tail call to '%s'\n", func_name);
                        return false;
                    }
                    {
                        const uint8_t *call_code = runtime->module->data + callee_fn->code_offset;
                        size_t call_code_size = (size_t)callee_fn->code_size;
                        size_t arg_start = *sp - (size_t)arg_count;
                        elx_value_t saved[ELX_VM_STACK_SIZE];
                        size_t i;
                        for (i = 0U; i < arg_start; ++i) saved[i] = stack[i];
                        *sp = arg_start;
                        memset(runtime->vm.registers, 0, runtime->vm.register_count * sizeof(elx_value_t));
                        if (!execute_bytecode(runtime, call_code, call_code_size, stack, sp, return_value)) {
                            return false;
                        }
                        *sp = arg_start;
                        for (i = 0U; i < arg_start; ++i) stack[i] = saved[i];
                    }
                    return true;
                }
                break;
            }
            case ELX_OP_RETURN: {
                if (pc < code_size && code[pc] == ELX_OP_RETURN_TAIL_FLAG) {
                    pc += 1U;
                }
                if (return_value && *sp > 0U) {
                    *return_value = stack[*sp - 1U];
                    (*sp)--;
                }
                return true;
            }
            case ELX_OP_JUMP: {
                if (pc + 4U > code_size) return false;
                index = read_u32(code, &pc);
                pc = index;
                break;
            }
            case ELX_OP_JUMP_IF_FALSE: {
                if (pc + 4U > code_size) return false;
                index = read_u32(code, &pc);
                if (*sp == 0U) return false;
                v = stack[*sp - 1U];
                if (v.type != ELX_VAL_BOOL || !v.boolean) {
                    pc = index;
                }
                break;
            }
            case ELX_OP_JUMP_IF_TRUE: {
                if (pc + 4U > code_size) return false;
                index = read_u32(code, &pc);
                if (*sp == 0U) return false;
                v = stack[*sp - 1U];
                if (v.type == ELX_VAL_BOOL && v.boolean) {
                    pc = index;
                }
                break;
            }
            case ELX_OP_LOOP_START:
                break;
            case ELX_OP_LOOP_END: {
                if (pc + 4U > code_size) return false;
                pc += 4U;
                break;
            }
            case ELX_OP_BINARY: {
                if (pc + 1U > code_size) return false;
                {
                    uint8_t op = code[pc++];
                    elx_value_t right, result;
                    if (*sp < 2U) {
                        fprintf(stderr, "elx_runtime: binary op stack underflow\n");
                        return false;
                    }
                    right = stack[*sp - 1U];
                    if (elx_binary_op((elx_binary_op_t)op, stack[*sp - 2U], right, &result) == 0) {
                        stack[*sp - 2U] = result;
                    } else {
                        fprintf(stderr, "elx_runtime: binary op failed\n");
                        return false;
                    }
                    (*sp)--;
                }
                break;
            }
            case ELX_OP_UNARY: {
                if (pc + 1U > code_size) return false;
                {
                    uint8_t op = code[pc++];
                    elx_value_t result;
                    if (*sp == 0U) {
                        fprintf(stderr, "elx_runtime: unary op stack underflow\n");
                        return false;
                    }
                    if (elx_unary_op((elx_unary_op_t)op, stack[*sp - 1U], &result) == 0) {
                        stack[*sp - 1U] = result;
                    } else {
                        fprintf(stderr, "elx_runtime: unary op failed\n");
                        return false;
                    }
                }
                break;
            }
            case ELX_OP_STACK_ALLOC:
                break;
            case ELX_OP_WIDGET_CREATE: {
                const char *name = NULL;
                if (pc + 4U > code_size) break;
                index = read_u32(code, &pc);
                if (runtime->module->string_count > 0U && index > 0U && index <= runtime->module->string_count) {
                    name = runtime->module->string_pool.strings[index - 1U];
                }
                /* WIDGET_CREATE: no-op in public SDK (no UI bridge) */
                (void)name;
                break;
            }
            case ELX_OP_WIDGET_PROPERTY: {
                const char *prop_name = NULL;
                const char *prop_value = NULL;
                if (pc + 4U > code_size) break;
                index = read_u32(code, &pc);
                if (runtime->module->string_count > 0U && index > 0U && index <= runtime->module->string_count) {
                    prop_name = runtime->module->string_pool.strings[index - 1U];
                }
                if (runtime->widget_stack_count > 0U) {
                    void *widget = runtime->widget_stack[runtime->widget_stack_count - 1U];
                    /* WIDGET_PROPERTY: no-op in public SDK (no UI bridge) */
                    (void)prop_name;
                    if (*sp > 0U) (*sp)--;
                }
                break;
            }
            case ELX_OP_BLOCK_END:
                break;
            case ELX_OP_STRING:
                break;
            case ELX_OP_MEMBER:
                break;
            case ELX_OP_INDEX:
                break;
            case ELX_OP_STATE_SUBSCRIBE:
                break;
            default:
                fprintf(stderr, "elx_runtime: unknown opcode 0x%02X at pc=%zu\n", opcode, pc - 1);
                return false;
        }
    }
    return true;
}

const char *elx_value_to_string(const elx_value_t *value) {
    static char buffer[64];
    if (!value) return "(null)";
    switch (value->type) {
        case ELX_VAL_INT:
            snprintf(buffer, sizeof(buffer), "%lld", (long long)value->integer);
            return buffer;
        case ELX_VAL_FLOAT:
            snprintf(buffer, sizeof(buffer), "%f", value->real);
            return buffer;
        case ELX_VAL_BOOL:
            return value->boolean ? "true" : "false";
        case ELX_VAL_STRING:
            return value->string ? value->string : "";
        case ELX_VAL_NIL:
            return "nil";
        default:
            return "(unknown)";
    }
}

void elx_native_register(elx_runtime_t *runtime, const char *name, uint32_t arg_count,
                         elx_value_t (*fn)(elx_value_t *args, uint32_t count)) {
    elx_native_fn_t *entry;
    size_t i;
    if (!runtime || !name || !fn) return;
    for (i = 0; i < runtime->natives.count; ++i) {
        if (strcmp(runtime->natives.items[i].name, name) == 0) {
            runtime->natives.items[i].arg_count = arg_count;
            runtime->natives.items[i].fn = fn;
            return;
        }
    }
    if (runtime->natives.count >= runtime->natives.capacity) {
        size_t new_cap = runtime->natives.capacity ? runtime->natives.capacity * 2 : 16;
        entry = (elx_native_fn_t *)realloc(runtime->natives.items, new_cap * sizeof(*entry));
        if (!entry) return;
        runtime->natives.items = entry;
        runtime->natives.capacity = new_cap;
    }
    entry = &runtime->natives.items[runtime->natives.count];
    entry->name = name;
    entry->arg_count = arg_count;
    entry->fn = fn;
    runtime->natives.count++;
}

const elx_native_fn_t *elx_native_lookup(const elx_runtime_t *runtime, const char *name) {
    size_t i;
    if (!runtime || !name) return NULL;
    for (i = 0; i < runtime->natives.count; ++i) {
        if (strcmp(runtime->natives.items[i].name, name) == 0) {
            return &runtime->natives.items[i];
        }
    }
    return NULL;
}

elx_value_t native_print(elx_value_t *args, uint32_t count) {
    if (count > 0 && args) {
        const char *s = elx_value_to_string(&args[0]);
        if (s) {
            printf("%s", s);
            if (count > 1) printf("\n");
        }
    }
    return elx_value_nil();
}

elx_value_t native_clock(elx_value_t *args, uint32_t count) {
    (void)args; (void)count;
    return elx_value_int((int64_t)time(NULL));
}

static bool execute_function(elx_runtime_t *runtime, const elx_function_t *func, elx_value_t *return_value) {
    const uint8_t *code_data;
    size_t code_size;
    elx_value_t stack[ELX_VM_STACK_SIZE];
    size_t sp = 0;
    if (!runtime || !func) return false;
    code_size = (size_t)func->code_size;
    code_data = runtime->module->data + func->code_offset;
    memset(runtime->vm.registers, 0, runtime->vm.register_count * sizeof(elx_value_t));
    runtime->running = true;
    return execute_bytecode(runtime, code_data, code_size, stack, &sp, return_value);
}

bool elx_runtime_create(elx_runtime_t *runtime, const char *elbc_path) {
    if (!runtime || !elbc_path) return false;
    memset(runtime, 0, sizeof(*runtime));
    runtime->module = (elx_module_t *)calloc(1U, sizeof(elx_module_t));
    if (!runtime->module) return false;
    if (!elx_module_load(elbc_path, runtime->module)) {
        free(runtime->module);
        runtime->module = NULL;
        return false;
    }
    elx_vm_init(&runtime->vm, runtime->module->register_count > 0U ? runtime->module->register_count : 1024U);
    elx_stack_init(&runtime->call_stack);
    runtime->widget_stack_count = 0U;
    runtime->natives.count = 0U;
    runtime->natives.capacity = 0U;
    runtime->natives.items = NULL;
    elx_stdlib_register(runtime);
    runtime->arena = (elx_arena_t *)calloc(1U, sizeof(elx_arena_t));
    if (!runtime->arena) {
        elx_module_destroy(runtime->module);
        free(runtime->module);
        elx_vm_destroy(&runtime->vm);
        elx_stack_destroy(&runtime->call_stack);
        runtime->module = NULL;
        return false;
    }
    if (!elx_arena_create(runtime->arena, ELX_ARENA_BLOCK_SIZE)) {
        elx_module_destroy(runtime->module);
        free(runtime->module);
        elx_vm_destroy(&runtime->vm);
        elx_stack_destroy(&runtime->call_stack);
        free(runtime->arena);
        runtime->module = NULL;
        runtime->arena = NULL;
        return false;
    }
    runtime->running = false;
    runtime->exit_code = 0;
    return true;
}

void elx_runtime_destroy(elx_runtime_t *runtime) {
    if (!runtime) return;
    runtime->widget_stack_count = 0U;
    elx_module_destroy(runtime->module);
    free(runtime->module);
    elx_vm_destroy(&runtime->vm);
    elx_stack_destroy(&runtime->call_stack);
    free(runtime->natives.items);
    runtime->natives.items = NULL;
    runtime->natives.count = 0U;
    runtime->natives.capacity = 0U;
    if (runtime->arena) {
        elx_arena_destroy(runtime->arena);
        free(runtime->arena);
    }
    runtime->module = NULL;
    runtime->arena = NULL;
    runtime->running = false;
}

int elx_runtime_run(elx_runtime_t *runtime, const char *entry_point) {
    elx_function_t *func;
    elx_value_t result;
    if (!runtime || !entry_point) return 1;
    runtime->running = true;
    runtime->exit_code = 0;
    func = elx_module_find_function(runtime->module, entry_point);
    if (!func) {
        fprintf(stderr, "elx_runtime: entry point '%s' not found\n", entry_point ? entry_point : "(null)");
        runtime->running = false;
        runtime->exit_code = 1;
        return 1;
    }
    if (!execute_function(runtime, func, &result)) {
        runtime->running = false;
        runtime->exit_code = 1;
        runtime->last_result = elx_value_nil();
        return 1;
    }
    runtime->running = false;
    runtime->last_result = result;
    if (result.type == ELX_VAL_INT) {
        printf("Result: %lld\n", (long long)result.integer);
    } else if (result.type == ELX_VAL_FLOAT) {
        printf("Result: %f\n", result.real);
    } else if (result.type == ELX_VAL_BOOL) {
        printf("Result: %s\n", result.boolean ? "true" : "false");
    } else if (result.type == ELX_VAL_STRING) {
        printf("Result: %s\n", result.string ? result.string : "(null)");
    } else {
        printf("Result: nil\n");
    }
    return runtime->exit_code;
}

bool elx_runtime_call_function(elx_runtime_t *runtime, const char *name) {
    elx_function_t *func;
    elx_value_t result;
    if (!runtime || !name) return false;
    func = elx_module_find_function(runtime->module, name);
    if (!func) {
        fprintf(stderr, "elx_runtime: function '%s' not found\n", name);
        return false;
    }
    return execute_function(runtime, func, &result);
}
