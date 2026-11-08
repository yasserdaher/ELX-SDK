#include "elx_runtime.h"
#include <stdlib.h>
#include <string.h>

elx_value_t elx_value_nil(void) {
    elx_value_t value;
    value.type = ELX_VAL_NIL;
    value.integer = 0;
    return value;
}

elx_value_t elx_value_bool(bool value) {
    elx_value_t result;
    result.type = ELX_VAL_BOOL;
    result.boolean = value;
    return result;
}

elx_value_t elx_value_int(int64_t value) {
    elx_value_t result;
    result.type = ELX_VAL_INT;
    result.integer = value;
    return result;
}

elx_value_t elx_value_float(double value) {
    elx_value_t result;
    result.type = ELX_VAL_FLOAT;
    result.real = value;
    return result;
}

elx_value_t elx_value_string(const char *value) {
    elx_value_t result;
    result.type = ELX_VAL_STRING;
    result.string = value;
    return result;
}

elx_value_t elx_value_copy(elx_value_t value) {
    return value;
}

void elx_value_destroy(elx_value_t value) {
    (void)value;
}

int elx_binary_op(elx_binary_op_t op, elx_value_t left, elx_value_t right, elx_value_t *result) {
    if (!result) return -1;
    switch (op) {
        case ELX_BIN_ADD:
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT) {
                result->type = ELX_VAL_INT;
                result->integer = left.integer + right.integer;
            } else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_FLOAT) {
                result->type = ELX_VAL_FLOAT;
                result->real = left.real + right.real;
            } else if (left.type == ELX_VAL_INT && right.type == ELX_VAL_FLOAT) {
                result->type = ELX_VAL_FLOAT;
                result->real = (double)left.integer + right.real;
            } else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_INT) {
                result->type = ELX_VAL_FLOAT;
                result->real = left.real + (double)right.integer;
            } else {
                result->type = ELX_VAL_NIL;
            }
            break;
        case ELX_BIN_SUB:
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT) {
                result->type = ELX_VAL_INT;
                result->integer = left.integer - right.integer;
            } else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_FLOAT) {
                result->type = ELX_VAL_FLOAT;
                result->real = left.real - right.real;
            } else {
                result->type = ELX_VAL_NIL;
            }
            break;
        case ELX_BIN_MUL:
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT) {
                result->type = ELX_VAL_INT;
                result->integer = left.integer * right.integer;
            } else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_FLOAT) {
                result->type = ELX_VAL_FLOAT;
                result->real = left.real * right.real;
            } else {
                result->type = ELX_VAL_NIL;
            }
            break;
        case ELX_BIN_DIV:
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT) {
                result->type = ELX_VAL_INT;
                result->integer = right.integer ? left.integer / right.integer : 0;
            } else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_FLOAT) {
                result->type = ELX_VAL_FLOAT;
                result->real = right.real ? left.real / right.real : 0.0;
            } else {
                result->type = ELX_VAL_NIL;
            }
            break;
        case ELX_BIN_MOD:
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT) {
                result->type = ELX_VAL_INT;
                result->integer = right.integer ? left.integer % right.integer : 0;
            } else {
                result->type = ELX_VAL_NIL;
            }
            break;
        case ELX_BIN_LT:
            result->type = ELX_VAL_BOOL;
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT)
                result->boolean = left.integer < right.integer;
            else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_FLOAT)
                result->boolean = left.real < right.real;
            break;
        case ELX_BIN_LE:
            result->type = ELX_VAL_BOOL;
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT)
                result->boolean = left.integer <= right.integer;
            else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_FLOAT)
                result->boolean = left.real <= right.real;
            break;
        case ELX_BIN_GT:
            result->type = ELX_VAL_BOOL;
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT)
                result->boolean = left.integer > right.integer;
            else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_FLOAT)
                result->boolean = left.real > right.real;
            break;
        case ELX_BIN_GE:
            result->type = ELX_VAL_BOOL;
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT)
                result->boolean = left.integer >= right.integer;
            else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_FLOAT)
                result->boolean = left.real >= right.real;
            break;
        case ELX_BIN_EQ:
            result->type = ELX_VAL_BOOL;
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT)
                result->boolean = left.integer == right.integer;
            else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_FLOAT)
                result->boolean = left.real == right.real;
            break;
        case ELX_BIN_NE:
            result->type = ELX_VAL_BOOL;
            if (left.type == ELX_VAL_INT && right.type == ELX_VAL_INT)
                result->boolean = left.integer != right.integer;
            else if (left.type == ELX_VAL_FLOAT && right.type == ELX_VAL_FLOAT)
                result->boolean = left.real != right.real;
            break;
        case ELX_BIN_AND:
            result->type = ELX_VAL_INT;
            result->integer = (left.integer && right.integer) ? 1 : 0;
            break;
        case ELX_BIN_OR:
            result->type = ELX_VAL_INT;
            result->integer = (left.integer || right.integer) ? 1 : 0;
            break;
        default:
            result->type = ELX_VAL_NIL;
            break;
    }
    return 0;
}

int elx_unary_op(elx_unary_op_t op, elx_value_t operand, elx_value_t *result) {
    if (!result) return -1;
    switch (op) {
        case ELX_UNARY_NOT:
            result->type = ELX_VAL_INT;
            result->integer = (operand.integer == 0) ? 1 : 0;
            break;
        case ELX_UNARY_NEG:
            if (operand.type == ELX_VAL_INT) {
                result->type = ELX_VAL_INT;
                result->integer = -operand.integer;
            } else if (operand.type == ELX_VAL_FLOAT) {
                result->type = ELX_VAL_FLOAT;
                result->real = -operand.real;
            } else {
                result->type = ELX_VAL_NIL;
            }
            break;
        default:
            result->type = ELX_VAL_NIL;
            break;
    }
    return 0;
}

void elx_vm_init(elx_vm_t *vm, size_t register_count) {
    if (!vm) return;
    if (register_count > ELX_VM_MAX_REGISTERS) register_count = ELX_VM_MAX_REGISTERS;
    vm->registers = (elx_value_t *)calloc(register_count, sizeof(elx_value_t));
    vm->register_count = register_count;
}

void elx_vm_destroy(elx_vm_t *vm) {
    if (!vm) return;
    free(vm->registers);
    vm->registers = NULL;
    vm->register_count = 0U;
}

elx_value_t *elx_vm_register(elx_vm_t *vm, uint32_t index) {
    if (!vm || !vm->registers || index >= vm->register_count) return NULL;
    return &vm->registers[index];
}
