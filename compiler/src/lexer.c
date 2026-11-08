/*
 * ELVI OS — VOLT 1.26 License Agreement (ELECTRON VISION)
 * See LICENSE-ELX at the project root for full terms.
 */
#include "elx.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static char source_at(const elx_lexer_t *lexer, size_t offset) {
    if (offset >= lexer->length) return '\0';
    return lexer->source[offset];
}

static void advance(elx_lexer_t *lexer) {
    char value = source_at(lexer, lexer->position);
    if (!value) return;
    ++lexer->position;
    if (value == '\n') {
        ++lexer->line;
        lexer->column = 1U;
    } else {
        ++lexer->column;
    }
}

static void skip_space(elx_lexer_t *lexer) {
    for (;;) {
        char value = source_at(lexer, lexer->position);
        if (value == ' ' || value == '\t' || value == '\r' || value == '\n') {
            advance(lexer);
            continue;
        }
        if (value == '/' && source_at(lexer, lexer->position + 1U) == '/') {
            advance(lexer);
            advance(lexer);
            while (source_at(lexer, lexer->position) != '\n' && source_at(lexer, lexer->position) != '\0') advance(lexer);
            continue;
        }
        if (value == '/' && source_at(lexer, lexer->position + 1U) == '*') {
            elx_location_t start = {lexer->line, lexer->column, lexer->position};
            advance(lexer);
            advance(lexer);
            while (!(source_at(lexer, lexer->position) == '*' && source_at(lexer, lexer->position + 1U) == '/') && source_at(lexer, lexer->position) != '\0') advance(lexer);
            if (source_at(lexer, lexer->position) == '\0') {
                elx_diagnostic_add(lexer->diagnostics, 101, "unterminated block comment", start);
            } else {
                advance(lexer);
                advance(lexer);
            }
            continue;
        }
        break;
    }
}

static bool ident_start(unsigned char value) {
    return (value == '_') || (value >= 0x80U) || isalpha(value);
}

static bool ident_continue(unsigned char value) {
    return ident_start(value) || isdigit(value);
}

static elx_token_type_t keyword_type(const char *text, size_t length) {
    typedef struct { const char *text; elx_token_type_t type; } keyword_t;
    static const keyword_t keywords[] = {
        {"fn", ELX_TOKEN_FN}, {"struct", ELX_TOKEN_STRUCT}, {"enum", ELX_TOKEN_ENUM}, {"let", ELX_TOKEN_LET}, {"const", ELX_TOKEN_CONST}, {"var", ELX_TOKEN_VAR}, {"if", ELX_TOKEN_IF}, {"else", ELX_TOKEN_ELSE}, {"match", ELX_TOKEN_MATCH}, {"for", ELX_TOKEN_FOR}, {"while", ELX_TOKEN_WHILE}, {"loop", ELX_TOKEN_LOOP}, {"return", ELX_TOKEN_RETURN}, {"import", ELX_TOKEN_IMPORT}, {"module", ELX_TOKEN_MODULE}, {"use", ELX_TOKEN_USE}, {"pub", ELX_TOKEN_PUB}, {"priv", ELX_TOKEN_PRIV}, {"export", ELX_TOKEN_EXPORT}, {"native", ELX_TOKEN_NATIVE}, {"ffi", ELX_TOKEN_FFI}, {"service", ELX_TOKEN_SERVICE}, {"async", ELX_TOKEN_ASYNC}, {"await", ELX_TOKEN_AWAIT}, {"true", ELX_TOKEN_TRUE}, {"false", ELX_TOKEN_FALSE}, {"nil", ELX_TOKEN_NIL}, {"as", ELX_TOKEN_AS}, {"where", ELX_TOKEN_WHERE}, {"and", ELX_TOKEN_AND}, {"or", ELX_TOKEN_OR}, {"in", ELX_TOKEN_IN}, {"on", ELX_TOKEN_ON}, {"state", ELX_TOKEN_STATE}, {"widget", ELX_TOKEN_WIDGET}, {"permissions", ELX_TOKEN_PERMISSIONS}, {"capability", ELX_TOKEN_CAPABILITY}, {"task", ELX_TOKEN_TASK}, {"select", ELX_TOKEN_SELECT}, {"break", ELX_TOKEN_BREAK}, {"continue", ELX_TOKEN_CONTINUE}, {"impl", ELX_TOKEN_IMPL}, {"type", ELX_TOKEN_TYPE}
    };
    size_t index;
    for (index = 0U; index < sizeof(keywords) / sizeof(keywords[0]); ++index) {
        if (strlen(keywords[index].text) == length && memcmp(keywords[index].text, text, length) == 0) return keywords[index].type;
    }
    return ELX_TOKEN_IDENT;
}

static elx_token_t make_token(elx_lexer_t *lexer, elx_token_type_t type, size_t start, size_t start_line, size_t start_column, size_t length) {
    elx_token_t token;
    memset(&token, 0, sizeof(token));
    token.type = type;
    token.text = lexer->source + start;
    token.length = length;
    token.location.line = start_line;
    token.location.column = start_column;
    token.location.offset = start;
    token.end.line = lexer->line;
    token.end.column = lexer->column;
    token.end.offset = lexer->position;
    return token;
}

static elx_token_t read_identifier(elx_lexer_t *lexer) {
    size_t start = lexer->position;
    size_t start_line = lexer->line;
    size_t start_column = lexer->column;
    unsigned char value = (unsigned char)source_at(lexer, lexer->position);
    elx_token_t token;
    while (ident_continue(value)) {
        advance(lexer);
        value = (unsigned char)source_at(lexer, lexer->position);
    }
    token = make_token(lexer, ELX_TOKEN_IDENT, start, start_line, start_column, lexer->position - start);
    token.type = keyword_type(token.text, token.length);
    return token;
}

static elx_token_t read_number(elx_lexer_t *lexer) {
    size_t start = lexer->position;
    size_t start_line = lexer->line;
    size_t start_column = lexer->column;
    elx_token_t token;
    const char *end;
    bool floating = false;
    if (source_at(lexer, lexer->position) == '0' && (source_at(lexer, lexer->position + 1U) == 'x' || source_at(lexer, lexer->position + 1U) == 'X')) {
        advance(lexer);
        advance(lexer);
        while (isxdigit((unsigned char)source_at(lexer, lexer->position))) advance(lexer);
        token = make_token(lexer, ELX_TOKEN_INT, start, start_line, start_column, lexer->position - start);
        if (token.length <= 2U) elx_diagnostic_add(lexer->diagnostics, 102, "invalid hexadecimal integer", token.location);
        errno = 0;
        token.integer = strtoull(token.text + 2, (char **)&end, 16);
        if (errno != 0) elx_diagnostic_add(lexer->diagnostics, 103, "integer literal out of range", token.location);
        return token;
    }
    while (isdigit((unsigned char)source_at(lexer, lexer->position))) advance(lexer);
    if (source_at(lexer, lexer->position) == '.' && isdigit((unsigned char)source_at(lexer, lexer->position + 1U))) {
        floating = true;
        advance(lexer);
        while (isdigit((unsigned char)source_at(lexer, lexer->position))) advance(lexer);
    }
    if (source_at(lexer, lexer->position) == 'e' || source_at(lexer, lexer->position) == 'E') {
        floating = true;
        advance(lexer);
        if (source_at(lexer, lexer->position) == '+' || source_at(lexer, lexer->position) == '-') advance(lexer);
        if (!isdigit((unsigned char)source_at(lexer, lexer->position))) elx_diagnostic_add(lexer->diagnostics, 104, "invalid exponent", token.location);
        while (isdigit((unsigned char)source_at(lexer, lexer->position))) advance(lexer);
    }
    token = make_token(lexer, floating ? ELX_TOKEN_FLOAT : ELX_TOKEN_INT, start, start_line, start_column, lexer->position - start);
    if (floating) {
        errno = 0;
        token.real = strtod(token.text, (char **)&end);
        if (errno != 0) elx_diagnostic_add(lexer->diagnostics, 103, "float literal out of range", token.location);
    } else {
        errno = 0;
        token.integer = strtoull(token.text, (char **)&end, 10);
        if (errno != 0) elx_diagnostic_add(lexer->diagnostics, 103, "integer literal out of range", token.location);
    }
    return token;
}

static char decode_escape(elx_lexer_t *lexer, elx_location_t location) {
    char value = source_at(lexer, lexer->position);
    advance(lexer);
    switch (value) {
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case '0': return '\0';
        case '\\': return '\\';
        case '"': return '"';
        case '\'': return '\'';
        default:
            elx_diagnostic_add(lexer->diagnostics, 105, "unknown string escape", location);
            return value;
    }
}

static elx_token_t read_string(elx_lexer_t *lexer, char quote, elx_token_type_t type) {
    size_t start = lexer->position;
    elx_location_t location = {lexer->line, lexer->column, lexer->position};
    elx_token_t token;
    size_t capacity = 32U;
    size_t length = 0U;
    char *decoded;
    advance(lexer);
    decoded = (char *)malloc(capacity);
    if (!decoded) {
            elx_diagnostic_add(lexer->diagnostics, 106, "out of memory while reading string", location);
            return make_token(lexer, type, start, location.line, location.column, 0U);
    }
    for (;;) {
        char value = source_at(lexer, lexer->position);
        if (value == '\0' || value == '\n') {
            elx_diagnostic_add(lexer->diagnostics, 107, "unterminated string literal", location);
            break;
        }
        if (value == quote) {
            advance(lexer);
            break;
        }
        if (value == '\\') {
            elx_location_t escape_location = {lexer->line, lexer->column, lexer->position};
            value = decode_escape(lexer, escape_location);
        } else {
            advance(lexer);
        }
        if (length + 1U >= capacity) {
            char *larger;
            capacity *= 2U;
            larger = (char *)realloc(decoded, capacity);
            if (!larger) {
                free(decoded);
                elx_diagnostic_add(lexer->diagnostics, 106, "out of memory while reading string", location);
                token = make_token(lexer, type, start, location.line, location.column, 0U);
                token.string = NULL;
                return token;
            }
            decoded = larger;
        }
        decoded[length++] = value;
    }
    decoded[length] = '\0';
    token = make_token(lexer, type, start, location.line, location.column, lexer->position - start);
    token.string = decoded;
    return token;
}

static elx_token_t read_operator(elx_lexer_t *lexer, char value) {
    size_t start = lexer->position;
    char next = source_at(lexer, lexer->position + 1U);
    elx_token_type_t type = ELX_TOKEN_UNKNOWN;
    switch (value) {
        case '+': type = next == '=' ? ELX_TOKEN_PLUS_ASSIGN : ELX_TOKEN_PLUS; break;
        case '-': type = next == '>' ? ELX_TOKEN_ARROW : (next == '=' ? ELX_TOKEN_MINUS_ASSIGN : ELX_TOKEN_MINUS); break;
        case '*': type = next == '=' ? ELX_TOKEN_STAR_ASSIGN : ELX_TOKEN_STAR; break;
        case '/': type = next == '=' ? ELX_TOKEN_SLASH_ASSIGN : ELX_TOKEN_SLASH; break;
        case '%': type = next == '=' ? ELX_TOKEN_PERCENT_ASSIGN : ELX_TOKEN_PERCENT; break;
        case '=': type = next == '=' ? ELX_TOKEN_EQ_EQ : (next == '>' ? ELX_TOKEN_FAT_ARROW : ELX_TOKEN_ASSIGN); break;
        case '!': type = next == '=' ? ELX_TOKEN_BANG_EQ : ELX_TOKEN_BANG; break;
        case '<': type = next == '=' ? ELX_TOKEN_LT_EQ : ELX_TOKEN_LT; break;
        case '>': type = next == '=' ? ELX_TOKEN_GT_EQ : ELX_TOKEN_GT; break;
        case '&': type = next == '&' ? ELX_TOKEN_AND_AND : ELX_TOKEN_UNKNOWN; break;
        case '|': type = next == '|' ? ELX_TOKEN_OR_OR : ELX_TOKEN_PIPE; break;
        case '.': type = next == '.' ? ELX_TOKEN_DOT_DOT : ELX_TOKEN_DOT; break;
        case ':': type = next == ':' ? ELX_TOKEN_DOUBLE_COLON : (next == '=' ? ELX_TOKEN_COLON_ASSIGN : ELX_TOKEN_COLON); break;
        case '(': type = ELX_TOKEN_LPAREN; break;
        case ')': type = ELX_TOKEN_RPAREN; break;
        case '{': type = ELX_TOKEN_LBRACE; break;
        case '}': type = ELX_TOKEN_RBRACE; break;
        case '[': type = ELX_TOKEN_LBRACKET; break;
        case ']': type = ELX_TOKEN_RBRACKET; break;
        case ',': type = ELX_TOKEN_COMMA; break;
        case ';': type = ELX_TOKEN_SEMICOLON; break;
        default: break;
    }
    if (type == ELX_TOKEN_UNKNOWN) {
        elx_diagnostic_add(lexer->diagnostics, 108, "unknown character", (elx_location_t){lexer->line, lexer->column, lexer->position});
        advance(lexer);
        return make_token(lexer, type, start, lexer->line, lexer->column - 1U, 1U);
    }
    advance(lexer);
    if (type == ELX_TOKEN_PLUS_ASSIGN || type == ELX_TOKEN_MINUS_ASSIGN || type == ELX_TOKEN_STAR_ASSIGN || type == ELX_TOKEN_SLASH_ASSIGN || type == ELX_TOKEN_PERCENT_ASSIGN || type == ELX_TOKEN_EQ_EQ || type == ELX_TOKEN_BANG_EQ || type == ELX_TOKEN_LT_EQ || type == ELX_TOKEN_GT_EQ || type == ELX_TOKEN_AND_AND || type == ELX_TOKEN_OR_OR || type == ELX_TOKEN_ARROW || type == ELX_TOKEN_FAT_ARROW || type == ELX_TOKEN_DOT_DOT || type == ELX_TOKEN_DOUBLE_COLON || type == ELX_TOKEN_COLON_ASSIGN) advance(lexer);
    return make_token(lexer, type, start, lexer->line, lexer->column - (lexer->position - start), lexer->position - start);
}

elx_token_t elx_lexer_next(elx_lexer_t *lexer) {
    elx_token_t token;
    char value;
    if (!lexer) {
        token.type = ELX_TOKEN_EOF;
        token.text = "";
        token.length = 0U;
        return token;
    }
    skip_space(lexer);
    value = source_at(lexer, lexer->position);
    if (value == '\0') return make_token(lexer, ELX_TOKEN_EOF, lexer->position, lexer->line, lexer->column, 0U);
    if (ident_start((unsigned char)value)) return read_identifier(lexer);
    if (isdigit((unsigned char)value)) return read_number(lexer);
    if (value == '"' || value == '\'') return read_string(lexer, value, value == '"' ? ELX_TOKEN_STRING : ELX_TOKEN_STRING_VIEW);
    return read_operator(lexer, value);
}
