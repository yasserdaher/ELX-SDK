/*
 * ELVI OS — VOLT 1.26 License Agreement (ELECTRON VISION)
 * See LICENSE at the project root for full terms.
 */
#include "elx.h"
#include <stdlib.h>
#include <string.h>

static elx_compile_options_t default_options(void) {
    elx_compile_options_t options = {false, false, true, NULL, NULL, NULL};
    return options;
}

static void dump_tokens(const char *source, size_t length, elx_diagnostic_list_t *diagnostics) {
    elx_lexer_t *lexer = elx_lexer_create(source, length, diagnostics);
    elx_token_t token;
    if (!lexer) return;
    do {
        token = elx_lexer_next(lexer);
        fprintf(stdout, "%zu:%zu %-12s %.*s\n", token.location.line, token.location.column, elx_token_type_name(token.type), (int)token.length, token.text);
        elx_token_destroy(&token);
    } while (token.type != ELX_TOKEN_EOF);
    elx_lexer_destroy(lexer);
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

bool elx_compile_source(const char *source, size_t length, const char *output_path, const elx_compile_options_t *options, elx_optimization_report_t *report, elx_diagnostic_list_t *diagnostics) {
    elx_ast_t *program;
    elx_optimization_report_t opt_report;
    elx_compile_options_t effective = options ? *options : default_options();
    bool ok;
    if (!source || !diagnostics) return false;
    program = elx_parse(source, length, diagnostics);
    if (!program || diagnostics->has_errors) {
        elx_ast_destroy(program);
        return false;
    }
    if (effective.dump_tokens) dump_tokens(source, length, diagnostics);
    if (effective.dump_ast) elx_ast_print(program, stdout);
    if (!elx_semantic_analyze(program, diagnostics)) {
        elx_ast_destroy(program);
        return false;
    }
    memset(&opt_report, 0, sizeof(opt_report));
    if (effective.optimize) {
        elx_optimize(program, &opt_report);
        if (report) *report = opt_report;
    } else if (report) {
        memset(report, 0, sizeof(*report));
        count_nodes(program, report);
    }
    ok = output_path ? elx_write_elbc(program, output_path, diagnostics) : true;
    elx_ast_destroy(program);
    return ok;
}

bool elx_compile_file(const char *input_path, const char *output_path, const elx_compile_options_t *options, elx_optimization_report_t *report, elx_diagnostic_list_t *diagnostics) {
    FILE *file;
    long length;
    char *source;
    bool ok;
    if (!input_path || !output_path || !diagnostics) return false;
    file = fopen(input_path, "rb");
    if (!file) {
        elx_diagnostic_add(diagnostics, 100, "cannot open input file", (elx_location_t){1U, 1U, 0U});
        return false;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        elx_diagnostic_add(diagnostics, 101, "cannot seek input file", (elx_location_t){1U, 1U, 0U});
        return false;
    }
    length = ftell(file);
    if (length < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        elx_diagnostic_add(diagnostics, 102, "cannot measure input file", (elx_location_t){1U, 1U, 0U});
        return false;
    }
    source = (char *)malloc((size_t)length + 1U);
    if (!source) {
        fclose(file);
        elx_diagnostic_add(diagnostics, 103, "out of memory while reading input", (elx_location_t){1U, 1U, 0U});
        return false;
    }
    if (length > 0 && fread(source, 1U, (size_t)length, file) != (size_t)length) {
        free(source);
        fclose(file);
        elx_diagnostic_add(diagnostics, 104, "cannot read input file", (elx_location_t){1U, 1U, 0U});
        return false;
    }
    source[length] = '\0';
    fclose(file);
    ok = elx_compile_source(source, (size_t)length, output_path, options, report, diagnostics);
    free(source);
    return ok;
}

const char *elx_version(void) {
    return "0.4.1-phase4";
}
