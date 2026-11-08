#define _GNU_SOURCE
#include "elx_runtime.h"
#include <stdlib.h>
#include <string.h>

elx_reader_t *elx_reader_create(const uint8_t *data, size_t size) {
    elx_reader_t *reader = (elx_reader_t *)calloc(1U, sizeof(*reader));
    if (!reader) return NULL;
    reader->data = data;
    reader->size = size;
    reader->cursor = 0U;
    return reader;
}

void elx_reader_destroy(elx_reader_t *reader) {
    free(reader);
}

bool elx_reader_read_u8(elx_reader_t *reader, uint8_t *out) {
    if (!reader || !out || reader->cursor + 1U > reader->size) return false;
    *out = reader->data[reader->cursor];
    reader->cursor += 1U;
    return true;
}

bool elx_reader_read_u16(elx_reader_t *reader, uint16_t *out) {
    uint8_t b0, b1;
    if (!reader || !out) return false;
    if (!elx_reader_read_u8(reader, &b0)) return false;
    if (!elx_reader_read_u8(reader, &b1)) return false;
    *out = (uint16_t)(b0 | (b1 << 8U));
    return true;
}

bool elx_reader_read_u32(elx_reader_t *reader, uint32_t *out) {
    uint8_t b0, b1, b2, b3;
    if (!reader || !out) return false;
    if (!elx_reader_read_u8(reader, &b0)) return false;
    if (!elx_reader_read_u8(reader, &b1)) return false;
    if (!elx_reader_read_u8(reader, &b2)) return false;
    if (!elx_reader_read_u8(reader, &b3)) return false;
    *out = (uint32_t)(b0 | (b1 << 8U) | (b2 << 16U) | (b3 << 24U));
    return true;
}

bool elx_reader_read_u64(elx_reader_t *reader, uint64_t *out) {
    uint8_t b0, b1, b2, b3, b4, b5, b6, b7;
    if (!reader || !out) return false;
    if (!elx_reader_read_u8(reader, &b0)) return false;
    if (!elx_reader_read_u8(reader, &b1)) return false;
    if (!elx_reader_read_u8(reader, &b2)) return false;
    if (!elx_reader_read_u8(reader, &b3)) return false;
    if (!elx_reader_read_u8(reader, &b4)) return false;
    if (!elx_reader_read_u8(reader, &b5)) return false;
    if (!elx_reader_read_u8(reader, &b6)) return false;
    if (!elx_reader_read_u8(reader, &b7)) return false;
    *out = (uint64_t)b0 | ((uint64_t)b1 << 8U) | ((uint64_t)b2 << 16U) | ((uint64_t)b3 << 24U) |
           ((uint64_t)b4 << 32U) | ((uint64_t)b5 << 40U) | ((uint64_t)b6 << 48U) | ((uint64_t)b7 << 56U);
    return true;
}

bool elx_reader_read_bytes(elx_reader_t *reader, void *out, size_t count) {
    if (!reader || !out) return false;
    if (reader->cursor + count > reader->size) return false;
    memcpy(out, reader->data + reader->cursor, count);
    reader->cursor += count;
    return true;
}

bool elx_reader_read_string(elx_reader_t *reader, char *buffer, size_t max_len) {
    uint32_t length;
    if (!reader || !buffer || max_len == 0U) return false;
    if (!elx_reader_read_u32(reader, &length)) return false;
    if (length >= max_len) return false;
    if (!elx_reader_read_bytes(reader, buffer, length)) return false;
    buffer[length] = '\0';
    return true;
}

static uint32_t crc32(const uint8_t *data, size_t length) {
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

static char *read_file(const char *path, size_t *out_size) {
    FILE *file;
    long size;
    char *data;
    if (!path || !out_size) return NULL;
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    size = ftell(file);
    if (size < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (char *)malloc((size_t)size);
    if (!data) {
        fclose(file);
        return NULL;
    }
    if ((size_t)size != fread(data, 1U, (size_t)size, file)) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return data;
}

bool elx_module_load(const char *path, elx_module_t *module) {
    char *file_data;
    size_t file_size;
    uint8_t magic[4];
    uint8_t version_major;
    uint8_t version_minor;
    uint16_t flags16;
    uint64_t timestamp;
     uint32_t reserved;
    uint32_t function_count;
    uint32_t widget_count;
    uint32_t string_count;
    uint32_t register_count;
    uint32_t import_count = 0U;
    uint32_t export_count = 0U;
    uint32_t checksum;
    uint32_t computed;
    size_t i;
    const uint8_t *code_data;
    if (!path || !module) return false;
    memset(module, 0, sizeof(*module));
    file_data = read_file(path, &file_size);
    if (!file_data) return false;
    elx_reader_t *reader = elx_reader_create((const uint8_t *)file_data, file_size);
    if (!reader) {
        free(file_data);
        return false;
    }
    if (!elx_reader_read_bytes(reader, magic, 4U) ||
        memcmp(magic, "ELXB", 4U) != 0) {
        fprintf(stderr, "elx_runtime: invalid magic header\n");
        elx_reader_destroy(reader);
        free(file_data);
        return false;
    }
    if (!elx_reader_read_u8(reader, &version_major)) {
        fprintf(stderr, "elx_runtime: cannot read version major\n");
        elx_reader_destroy(reader);
        free(file_data);
        return false;
    }
    if (!elx_reader_read_u8(reader, &version_minor)) {
        fprintf(stderr, "elx_runtime: cannot read version minor\n");
        elx_reader_destroy(reader);
        free(file_data);
        return false;
    }
    if (version_major != 3U || version_minor < 1U || version_minor > 2U) {
        fprintf(stderr, "elx_runtime: unsupported bytecode version %u.%u\n", version_major, version_minor);
        elx_reader_destroy(reader);
        free(file_data);
        return false;
    }
    if (!elx_reader_read_u16(reader, &flags16)) goto fail_read;
    if (!elx_reader_read_u64(reader, &timestamp)) goto fail_read;
    if (!elx_reader_read_u32(reader, &reserved)) goto fail_read;
    (void)timestamp;
    (void)reserved;
    if (!elx_reader_read_u32(reader, &function_count)) goto fail_read;
    if (!elx_reader_read_u32(reader, &widget_count)) goto fail_read;
    if (!elx_reader_read_u32(reader, &string_count)) goto fail_read;
    if (!elx_reader_read_u32(reader, &register_count)) goto fail_read;
    if (version_minor >= 2U) {
        if (!elx_reader_read_u32(reader, &import_count)) goto fail_read;
        if (!elx_reader_read_u32(reader, &export_count)) goto fail_read;
    }
    module->version = (uint16_t)((version_major << 8U) | version_minor);
    module->flags = flags16;
    module->function_count = function_count;
    module->widget_count = widget_count;
    module->string_count = string_count;
    module->register_count = register_count;
    module->import_count = import_count;
    module->export_count = export_count;
    module->functions = (elx_function_t *)calloc(function_count, sizeof(elx_function_t));
    if (!module->functions && function_count > 0U) goto fail_read;
    for (i = 0U; i < function_count; ++i) {
        char name[256];
        uint32_t arg_count;
        uint32_t start_slot;
        uint32_t code_size;
        elx_function_t *func = &module->functions[i];
        if (!elx_reader_read_string(reader, name, sizeof(name))) goto fail_read;
        func->name = (char *)malloc(strlen(name) + 1U);
        if (func->name) strcpy(func->name, name);
        if (!elx_reader_read_u32(reader, &arg_count)) goto fail_read;
        if (!elx_reader_read_u32(reader, &start_slot)) goto fail_read;
        if (version_minor >= 2U) {
            uint8_t is_exported = 0U;
            if (!elx_reader_read_u8(reader, &is_exported)) goto fail_read;
            func->is_exported = is_exported != 0U;
        }
        if (!elx_reader_read_u32(reader, &code_size)) goto fail_read;
        func->arg_count = arg_count;
        func->start_slot = start_slot;
        func->code_size = code_size;
        func->code_offset = reader->cursor;
        reader->cursor += code_size;
    }
    if (import_count > 0U) {
        module->imports = (elx_import_t *)calloc(import_count, sizeof(elx_import_t));
        if (!module->imports) goto fail_read;
        for (i = 0U; i < import_count; ++i) {
            char mod_name[256], imp_path[256], alias[256];
            if (!elx_reader_read_string(reader, mod_name, sizeof(mod_name))) goto fail_read;
            if (!elx_reader_read_string(reader, imp_path, sizeof(imp_path))) goto fail_read;
            if (!elx_reader_read_string(reader, alias, sizeof(alias))) goto fail_read;
            module->imports[i].module_name = strdup(mod_name);
            module->imports[i].import_path = strdup(imp_path);
            module->imports[i].alias = alias[0] ? strdup(alias) : NULL;
        }
    }
    code_data = reader->data;
    {
        uint32_t pool_count;
        if (!elx_reader_read_u32(reader, &pool_count)) goto fail_read;
        (void)pool_count;
    }
    for (i = 0U; i < string_count; ++i) {
        char buffer[256];
        if (!elx_reader_read_string(reader, buffer, sizeof(buffer))) goto fail_read;
        if (module->string_pool.count == module->string_pool.capacity) {
            size_t cap = module->string_pool.capacity ? module->string_pool.capacity * 2U : ELX_STRING_POOL_INIT;
            char **strings = (char **)realloc(module->string_pool.strings, cap * sizeof(*strings));
            if (!strings) goto fail_read;
            module->string_pool.strings = strings;
            module->string_pool.capacity = cap;
        }
        module->string_pool.strings[module->string_pool.count] = (char *)malloc(strlen(buffer) + 1U);
        if (module->string_pool.strings[module->string_pool.count]) {
            strcpy(module->string_pool.strings[module->string_pool.count], buffer);
        }
        ++module->string_pool.count;
    }
    if (export_count > 0U) {
        module->exports = (elx_export_t *)calloc(export_count, sizeof(elx_export_t));
        if (!module->exports) goto fail_read;
        for (i = 0U; i < export_count; ++i) {
            char name[256];
            uint32_t func_index;
            if (!elx_reader_read_string(reader, name, sizeof(name))) goto fail_read;
            if (!elx_reader_read_u32(reader, &func_index)) goto fail_read;
            module->exports[i].name = strdup(name);
            module->exports[i].function_index = func_index;
        }
    }
    computed = crc32(code_data, reader->cursor);
    if (!elx_reader_read_u32(reader, &checksum)) goto fail_read;
    if (computed != checksum) {
        fprintf(stderr, "elx_runtime: checksum mismatch (stored: %08x, computed: %08x)\n", checksum, computed);
    }
    module->code_start = 0U;
    module->data_size = file_size;
    module->data = (const uint8_t *)file_data;
    module->string_pool.strings = (char **)realloc(module->string_pool.strings, (module->string_pool.count + 1) * sizeof(*module->string_pool.strings));
    elx_reader_destroy(reader);
    return true;
fail_read:
    elx_reader_destroy(reader);
    free(file_data);
    elx_module_destroy(module);
    return false;
}

void elx_module_destroy(elx_module_t *module) {
    size_t i;
    if (!module) return;
    for (i = 0U; i < module->function_count; ++i) {
        free(module->functions[i].name);
    }
    free(module->functions);
    for (i = 0U; i < module->import_count; ++i) {
        free(module->imports[i].module_name);
        free(module->imports[i].import_path);
        free(module->imports[i].alias);
    }
    free(module->imports);
    for (i = 0U; i < module->export_count; ++i) {
        free(module->exports[i].name);
    }
    free(module->exports);
    for (i = 0U; i < module->string_pool.count; ++i) {
        free(module->string_pool.strings[i]);
    }
    free(module->string_pool.strings);
    free((void *)module->data);
    memset(module, 0, sizeof(*module));
}

elx_function_t *elx_module_find_function(const elx_module_t *module, const char *name) {
    size_t i;
    if (!module || !name) return NULL;
    for (i = 0U; i < module->function_count; ++i) {
        if (module->functions[i].name && strcmp(module->functions[i].name, name) == 0) {
            return &module->functions[i];
        }
    }
    return NULL;
}

const elx_export_t *elx_module_find_export(const elx_module_t *module, const char *name) {
    size_t i;
    if (!module || !name) return NULL;
    for (i = 0U; i < module->export_count; ++i) {
        if (module->exports[i].name && strcmp(module->exports[i].name, name) == 0) {
            return &module->exports[i];
        }
    }
    return NULL;
}

bool elx_module_link_import(const elx_module_t *module, const char *import_name,
                            elx_module_t *target, const char *target_name) {
    const elx_export_t *exp;
    (void)module;
    (void)import_name;
    if (!target || !target_name) return false;
    exp = elx_module_find_export(target, target_name);
    return exp != NULL;
}

static elx_module_t *find_module_by_name(const elx_module_registry_t *registry,
                                         const char *name) {
    size_t i;
    if (!registry || !name) return NULL;
    for (i = 0U; i < registry->count; ++i) {
        if (registry->modules[i]->name && strcmp(registry->modules[i]->name, name) == 0) {
            return registry->modules[i];
        }
    }
    return NULL;
}

static elx_module_t *find_module_by_import_path(const elx_module_registry_t *registry,
                                                  const char *import_path) {
    size_t i;
    if (!registry || !import_path) return NULL;
    for (i = 0U; i < registry->count; ++i) {
        if (registry->modules[i]->name && strcmp(registry->modules[i]->name, import_path) == 0) {
            return registry->modules[i];
        }
    }
    return NULL;
}

static bool has_import_cycle(const elx_module_registry_t *registry,
                             const elx_module_t *current,
                             const elx_module_t *target,
                             const elx_module_t **path, size_t *path_len) {
    size_t i;
    if (!registry || !current || !target) return false;
    if (current == target) return true;
    for (i = 0U; i < *path_len; ++i) {
        if (path[i] == current) return false;
    }
    path[*path_len] = current;
    ++(*path_len);
    for (i = 0U; i < current->import_count; ++i) {
        const char *imp_path = current->imports[i].module_name;
        if (!imp_path) imp_path = current->imports[i].import_path;
        elx_module_t *dep = find_module_by_import_path(registry, imp_path);
        if (dep) {
            if (has_import_cycle(registry, dep, target, path, path_len)) return true;
        }
    }
    return false;
}

bool elx_registry_load(elx_module_registry_t *registry, const char *path) {
    elx_module_t *module;
    elx_module_t *existing;
    if (!registry || !path) return false;
    existing = find_module_by_name(registry, path);
    if (existing) return true;
    if (registry->count == registry->capacity) {
        size_t cap = registry->capacity ? registry->capacity * 2U : 8U;
        elx_module_t **modules = (elx_module_t **)realloc(registry->modules, cap * sizeof(*modules));
        if (!modules) return false;
        registry->modules = modules;
        registry->capacity = cap;
    }
    module = (elx_module_t *)calloc(1U, sizeof(*module));
    if (!module) return false;
    if (!elx_module_load(path, module)) {
        free(module);
        return false;
    }
    module->name = strdup(path);
    registry->modules[registry->count++] = module;
    return true;
}

void elx_registry_destroy(elx_module_registry_t *registry) {
    size_t i;
    if (!registry) return;
    for (i = 0U; i < registry->count; ++i) {
        elx_module_destroy(registry->modules[i]);
        free(registry->modules[i]->name);
        free(registry->modules[i]);
    }
    free(registry->modules);
    registry->modules = NULL;
    registry->count = 0;
    registry->capacity = 0;
}

elx_function_t *elx_registry_resolve(const elx_module_registry_t *registry,
                                     const char *module_name, const char *symbol) {
    elx_module_t *module;
    size_t i;
    if (!registry || !module_name || !symbol) return NULL;
    module = find_module_by_name(registry, module_name);
    if (!module) return NULL;
    for (i = 0U; i < module->export_count; ++i) {
        if (module->exports[i].name && strcmp(module->exports[i].name, symbol) == 0) {
            return &module->functions[module->exports[i].function_index];
        }
    }
    return NULL;
}

bool elx_registry_link_all(elx_module_registry_t *registry,
                           elx_link_error_cb error_cb, void *user_data) {
    size_t i, j;
    const elx_module_t *cycle_path[256];
    size_t cycle_len = 0;
    if (!registry) return false;
    for (i = 0U; i < registry->count; ++i) {
        elx_module_t *module = registry->modules[i];
        for (j = 0U; j < module->import_count; ++j) {
            const char *imp_name = module->imports[j].module_name;
            if (!imp_name) imp_name = module->imports[j].import_path;
            if (!imp_name || !find_module_by_import_path(registry, imp_name)) {
                if (error_cb) {
                    char msg[512];
                    snprintf(msg, sizeof(msg), "cannot find module '%s' required by import",
                             imp_name ? imp_name : "(null)");
                    error_cb(310, msg, user_data);
                }
                return false;
            }
        }
    }
    for (i = 0U; i < registry->count; ++i) {
        cycle_len = 0;
        for (j = 0U; j < registry->count; ++j) {
            if (i == j) continue;
            if (has_import_cycle(registry, registry->modules[j],
                                 registry->modules[i], cycle_path, &cycle_len)) {
                if (error_cb) {
                    char msg[512];
                    snprintf(msg, sizeof(msg), "circular import dependency involving module '%s'",
                             registry->modules[i]->name ?
                             registry->modules[i]->name : "(unknown)");
                    error_cb(312, msg, user_data);
                }
                return false;
            }
        }
    }
    return true;
}
