/*
 * ELVI OS — VOLT 1.26 License Agreement (ELECTRON VISION)
 * See LICENSE-ELX at the project root for full terms.
 */
#include "elx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    elx_lexer_t *lexer;
    elx_token_t current;
    elx_token_t peek;
    bool have_peek;
    elx_diagnostic_list_t *diagnostics;
} elx_parser_t;

static char *copy_string(const char *value, size_t length) {
    char *result;
    if (!value) return NULL;
    result = (char *)malloc(length + 1U);
    if (!result) return NULL;
    memcpy(result, value, length);
    result[length] = '\0';
    return result;
}

static void parser_error(elx_parser_t *parser, int code, const char *message, elx_location_t location) {
    elx_diagnostic_add(parser->diagnostics, code, message, location);
}

static void parser_advance(elx_parser_t *parser) {
    if (parser->have_peek) {
        elx_token_destroy(&parser->current);
        parser->current = parser->peek;
        parser->have_peek = false;
    } else {
        elx_token_destroy(&parser->current);
        parser->current = elx_lexer_next(parser->lexer);
    }
}

static elx_token_t parser_peek(elx_parser_t *parser) {
    if (!parser->have_peek) {
        parser->peek = elx_lexer_next(parser->lexer);
        parser->have_peek = true;
    }
    return parser->peek;
}

static bool parser_check(elx_parser_t *parser, elx_token_type_t type) {
    return parser->current.type == type;
}

static bool parser_match(elx_parser_t *parser, elx_token_type_t type) {
    if (!parser_check(parser, type)) return false;
    parser_advance(parser);
    return true;
}

static elx_token_t parser_expect(elx_parser_t *parser, elx_token_type_t type, int code, const char *message) {
    elx_token_t token;
    if (parser_check(parser, type)) {
        token = parser->current;
        parser_advance(parser);
        return token;
    }
    parser_error(parser, code, message, parser->current.location);
    token = parser->current;
    parser_advance(parser);
    return token;
}

static elx_type_t *parse_type(elx_parser_t *parser);
static elx_ast_t *parse_expression(elx_parser_t *parser, int min_precedence);
static elx_ast_t *parse_block(elx_parser_t *parser);
static elx_ast_t *parse_statement(elx_parser_t *parser);
static elx_ast_t *parse_declaration(elx_parser_t *parser);
static elx_ast_t *parse_import(elx_parser_t *parser);
static elx_ast_t *parse_export(elx_parser_t *parser);
static elx_ast_t *parse_module_decl(elx_parser_t *parser);

static elx_ast_t *parse_identifier(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    if (!parser_check(parser, ELX_TOKEN_IDENT)) {
        parser_error(parser, 200, "expected identifier", parser->current.location);
        parser_advance(parser);
        return NULL;
    }
    token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_IDENT, token.location);
    if (!node) return NULL;
    node->name = copy_string(token.text, token.length);
    node->end = token.end;
    return node;
}

static elx_ast_t *parse_struct_literal(elx_parser_t *parser) {
    elx_ast_t *literal;
    elx_token_t token;
    token = parser->current;
    literal = elx_ast_new(ELX_AST_STRUCT_LITERAL, token.location);
    if (!literal) return NULL;
    literal->name = copy_string(token.text, token.length);
    parser_advance(parser);
    parser_expect(parser, ELX_TOKEN_LBRACE, 218, "expected '{' to start struct literal");
    while (!parser_check(parser, ELX_TOKEN_RBRACE) && parser->current.type != ELX_TOKEN_EOF) {
        elx_ast_t *field;
        if (parser->current.type != ELX_TOKEN_IDENT) {
            parser_error(parser, 219, "expected field name in struct literal", parser->current.location);
            parser_advance(parser);
            continue;
        }
        field = elx_ast_new(ELX_AST_UI_PROPERTY, parser->current.location);
        field->name = copy_string(parser->current.text, parser->current.length);
        parser_advance(parser);
        parser_expect(parser, ELX_TOKEN_COLON, 220, "expected ':' after struct field name");
        field->value = parse_expression(parser, 0);
        field->end = field->value ? field->value->end : parser->current.location;
        field->next = literal->properties;
        literal->properties = field;
        parser_match(parser, ELX_TOKEN_COMMA);
    }
    parser_expect(parser, ELX_TOKEN_RBRACE, 221, "expected '}' to end struct literal");
    literal->end = parser->current.location;
    return literal;
}

static elx_ast_t *parse_widget_property(elx_parser_t *parser, elx_ast_t *widget) {
    elx_ast_t *property;
    elx_token_t token;
    token = parser->current;
    property = elx_ast_new(ELX_AST_UI_PROPERTY, token.location);
    if (!property) return NULL;
    property->name = copy_string(token.text, token.length);
    parser_advance(parser);
    parser_expect(parser, ELX_TOKEN_COLON, 213, "expected ':' after widget property name");
    property->value = parse_expression(parser, 0);
    property->end = property->value ? property->value->end : token.end;
    property->next = widget->properties;
    widget->properties = property;
    return property;
}

static elx_ast_t *parse_widget_event(elx_parser_t *parser, elx_ast_t *widget) {
    elx_ast_t *event;
    elx_token_t token;
    token = parser->current;
    event = elx_ast_new(ELX_AST_UI_EVENT, token.location);
    if (!event) return NULL;
    parser_advance(parser);
    event->name = copy_string(parser->current.text, parser->current.length);
    parser_advance(parser);
    parser_expect(parser, ELX_TOKEN_FAT_ARROW, 214, "expected '=>' after widget event");
    event->value = parse_block(parser);
    event->end = event->value ? event->value->end : token.end;
    event->next = widget->properties;
    widget->properties = event;
    return event;
}

static elx_ast_t *parse_widget(elx_parser_t *parser) {
    elx_ast_t *widget;
    elx_token_t token;
    token = parser->current;
    widget = elx_ast_new(ELX_AST_WIDGET, token.location);
    if (!widget) return NULL;
    widget->name = copy_string(token.text, token.length);
    parser_advance(parser);
    parser_expect(parser, ELX_TOKEN_LBRACE, 215, "expected '{' to start widget");
    while (!parser_check(parser, ELX_TOKEN_RBRACE) && parser->current.type != ELX_TOKEN_EOF) {
        if (parser->current.type == ELX_TOKEN_IDENT && parser_peek(parser).type == ELX_TOKEN_COLON) {
            parse_widget_property(parser, widget);
            parser_match(parser, ELX_TOKEN_COMMA);
        } else if (parser->current.type == ELX_TOKEN_ON) {
            parse_widget_event(parser, widget);
            parser_match(parser, ELX_TOKEN_COMMA);
        } else if (parser->current.type == ELX_TOKEN_IDENT) {
            elx_ast_t *child = parse_widget(parser);
            child->next = widget->children;
            widget->children = child;
            parser_match(parser, ELX_TOKEN_COMMA);
        } else {
            parser_error(parser, 216, "unexpected token in widget body", parser->current.location);
            parser_advance(parser);
        }
    }
    parser_expect(parser, ELX_TOKEN_RBRACE, 217, "expected '}' to end widget");
    widget->end = parser->current.location;
    return widget;
}

static elx_ast_t *parse_primary(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    switch (parser->current.type) {
        case ELX_TOKEN_INT:
            token = parser->current;
            parser_advance(parser);
            node = elx_ast_new(ELX_AST_INT, token.location);
            if (node) {
                node->integer = token.integer;
                node->end = token.end;
            }
            return node;
        case ELX_TOKEN_FLOAT:
            token = parser->current;
            parser_advance(parser);
            node = elx_ast_new(ELX_AST_FLOAT, token.location);
            if (node) {
                node->real = token.real;
                node->end = token.end;
            }
            return node;
        case ELX_TOKEN_STRING:
        case ELX_TOKEN_STRING_VIEW:
            token = parser->current;
            node = elx_ast_new(ELX_AST_STRING, token.location);
            if (node) {
                node->text = copy_string(token.string ? token.string : "", token.string ? strlen(token.string) : 0U);
                node->end = token.end;
            }
            parser_advance(parser);
            return node;
        case ELX_TOKEN_TRUE:
        case ELX_TOKEN_FALSE:
            token = parser->current;
            parser_advance(parser);
            node = elx_ast_new(ELX_AST_BOOL, token.location);
            if (node) {
                node->boolean = token.type == ELX_TOKEN_TRUE;
                node->end = token.end;
            }
            return node;
        case ELX_TOKEN_NIL:
            token = parser->current;
            parser_advance(parser);
            node = elx_ast_new(ELX_AST_NIL, token.location);
            if (node) node->end = token.end;
            return node;
        case ELX_TOKEN_IDENT:
            if (parser_peek(parser).type == ELX_TOKEN_LBRACE &&
                parser->current.text && parser->current.length > 0 &&
                parser->current.text[0] >= 'A' && parser->current.text[0] <= 'Z') {
                return parse_struct_literal(parser);
            }
            return parse_identifier(parser);
        case ELX_TOKEN_LPAREN:
            parser_advance(parser);
            node = parse_expression(parser, 0);
            parser_expect(parser, ELX_TOKEN_RPAREN, 201, "expected ')' after expression");
            if (node) node->end = parser->current.location;
            return node;
        case ELX_TOKEN_FN:
            parser_advance(parser);
            node = elx_ast_new(ELX_AST_LAMBDA, parser->current.location);
            if (!node) return NULL;
            if (parser_match(parser, ELX_TOKEN_LPAREN)) {
                elx_ast_t *parameters = NULL;
                if (!parser_check(parser, ELX_TOKEN_RPAREN)) {
                    do {
                        elx_ast_t *parameter = parse_identifier(parser);
                        if (parameter && parser_match(parser, ELX_TOKEN_COLON)) {
                            parameter->type = parse_type(parser);
                        }
                        parameters = elx_ast_append(parameters, parameter);
                    } while (parser_match(parser, ELX_TOKEN_COMMA));
                }
                parser_expect(parser, ELX_TOKEN_RPAREN, 202, "expected ')' after lambda parameters");
                node->args = parameters;
            }
            node->body = parse_block(parser);
            node->end = node->body ? node->body->end : node->location;
            return node;
        default:
            parser_error(parser, 203, "expected expression", parser->current.location);
            parser_advance(parser);
            return NULL;
    }
}

static int binary_precedence(elx_token_type_t type) {
    switch (type) {
        case ELX_TOKEN_OR:
        case ELX_TOKEN_OR_OR: return 1;
        case ELX_TOKEN_AND:
        case ELX_TOKEN_AND_AND: return 2;
        case ELX_TOKEN_EQ:
        case ELX_TOKEN_EQ_EQ:
        case ELX_TOKEN_BANG_EQ: return 3;
        case ELX_TOKEN_LT:
        case ELX_TOKEN_LT_EQ:
        case ELX_TOKEN_GT:
        case ELX_TOKEN_GT_EQ: return 4;
        case ELX_TOKEN_PLUS:
        case ELX_TOKEN_MINUS: return 5;
        case ELX_TOKEN_STAR:
        case ELX_TOKEN_SLASH:
        case ELX_TOKEN_PERCENT: return 6;
        default: return -1;
    }
}

static elx_ast_t *parse_unary(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    if (parser->current.type == ELX_TOKEN_BANG || parser->current.type == ELX_TOKEN_MINUS) {
        token = parser->current;
        parser_advance(parser);
        node = elx_ast_new(ELX_AST_UNARY, token.location);
        if (node) {
            node->operator = token.type;
            node->left = parse_unary(parser);
            node->end = node->left ? node->left->end : token.end;
        }
        return node;
    }
    return parse_primary(parser);
}

static elx_ast_t *parse_postfix(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_ast_t *member;
    elx_ast_t *arg;
    node = parse_unary(parser);
    for (;;) {
        if (parser_match(parser, ELX_TOKEN_DOT)) {
            member = elx_ast_new(ELX_AST_MEMBER, parser->current.location);
            if (!member) return node;
            member->object = node;
            member->name = copy_string(parser->current.text, parser->current.length);
            parser_advance(parser);
            member->end = parser->current.location;
            node = member;
        } else if (parser_match(parser, ELX_TOKEN_LPAREN)) {
            elx_ast_t *call = elx_ast_new(ELX_AST_CALL, node->location);
            if (!call) return node;
            call->callee = node;
            if (node->kind == ELX_AST_IDENT && node->name) {
                call->func_name = node->name;
            } else if (node->kind == ELX_AST_MEMBER) {
                /* Build a dotted function name for member-based calls
                   such as std.fs.read_text(...). */
                elx_ast_t *chain[8];
                int nchain = 0;
                elx_ast_t *tmp = node;
                while (tmp && tmp->kind == ELX_AST_MEMBER && nchain < 8) {
                    chain[nchain++] = tmp;
                    tmp = tmp->object;
                }
                if (tmp && tmp->kind == ELX_AST_IDENT && tmp->name && nchain > 0) {
                    /* Build name: ident.member1.member2... */
                    size_t total = strlen(tmp->name);
                    for (int i = nchain - 1; i >= 0; i--) {
                        total += 1 + strlen(chain[i]->name);
                    }
                    char *name = (char *)calloc(total + 1, 1);
                    if (name) {
                        strcpy(name, tmp->name);
                        for (int i = nchain - 1; i >= 0; i--) {
                            strcat(name, ".");
                            strncat(name, chain[i]->name, strlen(chain[i]->name));
                        }
                        call->func_name = name;
                    }
                }
            }
            if (!parser_check(parser, ELX_TOKEN_RPAREN)) {
                do {
                    arg = parse_expression(parser, 0);
                    call->args = elx_ast_append(call->args, arg);
                } while (parser_match(parser, ELX_TOKEN_COMMA));
            }
            parser_expect(parser, ELX_TOKEN_RPAREN, 204, "expected ')' after function arguments");
            call->end = parser->current.location;
            node = call;
        } else if (parser_match(parser, ELX_TOKEN_DOUBLE_COLON)) {
            parser_advance(parser);
            if (parser_match(parser, ELX_TOKEN_IDENT)) {
                elx_ast_t *call = elx_ast_new(ELX_AST_CALL, node->location);
                if (!call) return node;
                call->callee = node;
                if (parser_match(parser, ELX_TOKEN_LPAREN)) {
                    if (!parser_check(parser, ELX_TOKEN_RPAREN)) {
                        do {
                            arg = parse_expression(parser, 0);
                            call->args = elx_ast_append(call->args, arg);
                        } while (parser_match(parser, ELX_TOKEN_COMMA));
                    }
                    parser_expect(parser, ELX_TOKEN_RPAREN, 204, "expected ')' after function arguments");
                }
                call->end = parser->current.location;
                node = call;
            }
        } else if (parser_match(parser, ELX_TOKEN_LBRACKET)) {
            elx_ast_t *index = elx_ast_new(ELX_AST_INDEX, node->location);
            if (!index) return node;
            index->object = node;
            index->value = parse_expression(parser, 0);
            parser_expect(parser, ELX_TOKEN_RBRACKET, 205, "expected ']' after index");
            index->end = parser->current.location;
            node = index;
        } else {
            break;
        }
    }
    return node;
}

static elx_ast_t *parse_expression(elx_parser_t *parser, int min_precedence) {
    elx_ast_t *left;
    left = parse_postfix(parser);
    for (;;) {
        elx_token_type_t type = parser->current.type;
        int precedence = binary_precedence(type);
        elx_ast_t *right;
        if (precedence < min_precedence) break;
        parser_advance(parser);
        right = parse_expression(parser, precedence + 1);
        {
            elx_ast_t *binary = elx_ast_new(ELX_AST_BINARY, left ? left->location : parser->current.location);
            if (!binary) return left;
            binary->operator = type;
            binary->left = left;
            binary->right = right;
            binary->end = right ? right->end : parser->current.location;
            left = binary;
        }
    }
    return left;
}

static elx_ast_t *parse_assignment(elx_parser_t *parser) {
    elx_ast_t *left;
    elx_token_t operator;
    elx_ast_t *right;
    elx_ast_t *assign;
    left = parse_expression(parser, 0);
    if (parser->current.type == ELX_TOKEN_ASSIGN || parser->current.type == ELX_TOKEN_PLUS_ASSIGN ||
        parser->current.type == ELX_TOKEN_MINUS_ASSIGN || parser->current.type == ELX_TOKEN_STAR_ASSIGN ||
        parser->current.type == ELX_TOKEN_SLASH_ASSIGN || parser->current.type == ELX_TOKEN_PERCENT_ASSIGN) {
        operator = parser->current;
        parser_advance(parser);
        right = parse_expression(parser, 0);
        assign = elx_ast_new(ELX_AST_ASSIGN, operator.location);
        if (!assign) return left;
        assign->operator = operator.type;
        assign->left = left;
        assign->right = right;
        assign->end = right ? right->end : operator.end;
        return assign;
    }
    return left;
}

static elx_ast_t *parse_if(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_IF, token.location);
    if (!node) return NULL;
    node->condition = parse_expression(parser, 0);
    node->body = parse_block(parser);
    if (parser_match(parser, ELX_TOKEN_ELSE)) {
        if (parser_match(parser, ELX_TOKEN_IF)) {
            node->else_body = parse_if(parser);
        } else {
            node->else_body = parse_block(parser);
        }
    }
    node->end = node->else_body ? node->else_body->end : (node->body ? node->body->end : token.end);
    return node;
}

static elx_ast_t *parse_match_arm(elx_parser_t *parser) {
    elx_ast_t *arm;
    elx_token_t token;
    token = parser->current;
    arm = elx_ast_new(ELX_AST_MATCH_ARM, token.location);
    if (!arm) return NULL;
    arm->left = parse_expression(parser, 0);
    parser_expect(parser, ELX_TOKEN_FAT_ARROW, 206, "expected '=>' in match arm");
    arm->right = parse_expression(parser, 0);
    arm->end = arm->right ? arm->right->end : token.end;
    return arm;
}

static elx_ast_t *parse_match(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_MATCH, token.location);
    if (!node) return NULL;
    node->condition = parse_expression(parser, 0);
    parser_expect(parser, ELX_TOKEN_LBRACE, 207, "expected '{' to start match");
    while (!parser_check(parser, ELX_TOKEN_RBRACE) && parser->current.type != ELX_TOKEN_EOF) {
        elx_ast_t *arm = parse_match_arm(parser);
        node->items = elx_ast_append(node->items, arm);
        parser_match(parser, ELX_TOKEN_COMMA);
        parser_match(parser, ELX_TOKEN_SEMICOLON);
    }
    parser_expect(parser, ELX_TOKEN_RBRACE, 208, "expected '}' to end match");
    node->end = parser->current.location;
    return node;
}

static elx_ast_t *parse_for(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_FOR, token.location);
    if (!node) return NULL;
    node->target = parse_identifier(parser);
    parser_expect(parser, ELX_TOKEN_IN, 209, "expected 'in' in for loop");
    node->condition = parse_expression(parser, 0);
    node->body = parse_block(parser);
    node->end = node->body ? node->body->end : token.end;
    return node;
}

static elx_ast_t *parse_while(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_WHILE, token.location);
    if (!node) return NULL;
    node->condition = parse_expression(parser, 0);
    node->body = parse_block(parser);
    node->end = node->body ? node->body->end : token.end;
    return node;
}

static elx_ast_t *parse_loop(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_LOOP, token.location);
    if (!node) return NULL;
    node->body = parse_block(parser);
    node->end = node->body ? node->body->end : token.end;
    return node;
}

static elx_ast_t *parse_return(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_RETURN, token.location);
    if (!node) return NULL;
    if (!parser_check(parser, ELX_TOKEN_SEMICOLON) && !parser_check(parser, ELX_TOKEN_RBRACE) && parser->current.type != ELX_TOKEN_EOF) {
        node->value = parse_expression(parser, 0);
    }
    node->end = node->value ? node->value->end : token.end;
    return node;
}

static elx_ast_t *parse_break(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_BREAK, token.location);
    if (node) node->end = token.end;
    return node;
}

static elx_ast_t *parse_continue(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_CONTINUE, token.location);
    if (node) node->end = token.end;
    return node;
}

static elx_ast_t *parse_statement(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    switch (parser->current.type) {
        case ELX_TOKEN_IF: return parse_if(parser);
        case ELX_TOKEN_MATCH: return parse_match(parser);
        case ELX_TOKEN_FOR: return parse_for(parser);
        case ELX_TOKEN_WHILE: return parse_while(parser);
        case ELX_TOKEN_LOOP: return parse_loop(parser);
        case ELX_TOKEN_RETURN: return parse_return(parser);
        case ELX_TOKEN_BREAK: return parse_break(parser);
        case ELX_TOKEN_CONTINUE: return parse_continue(parser);
        case ELX_TOKEN_LET:
        case ELX_TOKEN_CONST:
        case ELX_TOKEN_VAR:
            token = parser->current;
            parser_advance(parser);
            node = elx_ast_new(token.type == ELX_TOKEN_LET ? ELX_AST_VAR : (token.type == ELX_TOKEN_CONST ? ELX_AST_CONST : ELX_AST_VAR), token.location);
            if (!node) return NULL;
            node->mutable_value = token.type != ELX_TOKEN_CONST;
            node->name = copy_string(parser->current.text, parser->current.length);
            parser_advance(parser);
            if (parser_match(parser, ELX_TOKEN_COLON)) {
                node->type = parse_type(parser);
            }
            parser_expect(parser, ELX_TOKEN_ASSIGN, 210, "expected '=' after variable declaration");
            node->value = parse_assignment(parser);
            node->end = node->value ? node->value->end : token.end;
            return node;
        case ELX_TOKEN_IDENT:
            if (parser_peek(parser).type == ELX_TOKEN_LBRACE) {
                return parse_widget(parser);
            }
            node = elx_ast_new(ELX_AST_EXPR_STMT, parser->current.location);
            if (!node) return NULL;
            node->value = parse_assignment(parser);
            node->end = node->value ? node->value->end : parser->current.location;
            return node;
        case ELX_TOKEN_LBRACE:
            return parse_block(parser);
        default:
            node = elx_ast_new(ELX_AST_EXPR_STMT, parser->current.location);
            if (!node) return NULL;
            node->value = parse_assignment(parser);
            node->end = node->value ? node->value->end : parser->current.location;
            return node;
    }
}

static elx_ast_t *parse_block(elx_parser_t *parser) {
    elx_ast_t *block;
    elx_ast_t *tail;
    elx_token_t token;
    token = parser->current;
    parser_expect(parser, ELX_TOKEN_LBRACE, 211, "expected '{' to start block");
    block = elx_ast_new(ELX_AST_BLOCK, token.location);
    if (!block) return NULL;
    tail = NULL;
    while (!parser_check(parser, ELX_TOKEN_RBRACE) && parser->current.type != ELX_TOKEN_EOF) {
        elx_ast_t *statement = parse_statement(parser);
        if (statement) {
            if (tail) {
                tail->next = statement;
            } else {
                block->next = statement;
            }
            tail = statement;
        }
        parser_match(parser, ELX_TOKEN_SEMICOLON);
    }
    parser_expect(parser, ELX_TOKEN_RBRACE, 212, "expected '}' to end block");
    block->end = parser->current.location;
    return block;
}

static elx_type_t *parse_type(elx_parser_t *parser) {
    elx_type_t *type;
    elx_token_t token;
    char *name;
    if (parser->current.type == ELX_TOKEN_IDENT) {
        token = parser->current;
        parser_advance(parser);
        name = copy_string(token.text, token.length);
        type = elx_type_named(name);
        free(name);
        if (parser_match(parser, ELX_TOKEN_LT)) {
            while (!parser_check(parser, ELX_TOKEN_GT) && parser->current.type != ELX_TOKEN_EOF) {
                elx_type_t *parameter = parse_type(parser);
                if (parameter) {
                    type->parameters = (elx_type_t **)realloc(type->parameters, (type->parameter_count + 1U) * sizeof(*type->parameters));
                    type->parameters[type->parameter_count++] = parameter;
                }
                parser_match(parser, ELX_TOKEN_COMMA);
            }
            parser_expect(parser, ELX_TOKEN_GT, 222, "expected '>' to end type parameters");
        }
        return type;
    }
    parser_error(parser, 223, "expected type name", parser->current.location);
    parser_advance(parser);
    return elx_type_new(ELX_TYPE_INVALID);
}

static elx_ast_t *parse_function(elx_parser_t *parser) {
    elx_ast_t *function;
    elx_token_t token;
    elx_ast_t *parameter;
    token = parser->current;
    parser_advance(parser);
    function = elx_ast_new(ELX_AST_FUNCTION, token.location);
    if (!function) return NULL;
    function->name = copy_string(parser->current.text, parser->current.length);
    parser_advance(parser);
    parser_expect(parser, ELX_TOKEN_LPAREN, 224, "expected '(' after function name");
    if (!parser_check(parser, ELX_TOKEN_RPAREN)) {
        do {
            parameter = parse_identifier(parser);
            if (parameter && parser_match(parser, ELX_TOKEN_COLON)) {
                parameter->type = parse_type(parser);
            }
            function->args = elx_ast_append(function->args, parameter);
        } while (parser_match(parser, ELX_TOKEN_COMMA));
    }
    parser_expect(parser, ELX_TOKEN_RPAREN, 225, "expected ')' after function parameters");
    if (parser_match(parser, ELX_TOKEN_ARROW)) {
        function->type = parse_type(parser);
    }
    function->body = parse_block(parser);
    function->end = function->body ? function->body->end : token.end;
    return function;
}

static elx_ast_t *parse_state(elx_parser_t *parser) {
    elx_ast_t *state;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    state = elx_ast_new(ELX_AST_STATE, token.location);
    if (!state) return NULL;
    state->name = copy_string(parser->current.text, parser->current.length);
    parser_advance(parser);
    parser_expect(parser, ELX_TOKEN_LBRACE, 226, "expected '{' to start state");
    while (!parser_check(parser, ELX_TOKEN_RBRACE) && parser->current.type != ELX_TOKEN_EOF) {
        elx_ast_t *field;
        if (parser->current.type != ELX_TOKEN_IDENT) {
            parser_error(parser, 227, "expected field name in state", parser->current.location);
            parser_advance(parser);
            continue;
        }
        field = elx_ast_new(ELX_AST_VAR, parser->current.location);
        field->name = copy_string(parser->current.text, parser->current.length);
        parser_advance(parser);
        if (parser_match(parser, ELX_TOKEN_COLON)) {
            field->type = parse_type(parser);
        }
        if (parser_match(parser, ELX_TOKEN_ASSIGN)) {
            field->value = parse_expression(parser, 0);
        }
        field->end = field->value ? field->value->end : parser->current.location;
        field->next = state->items;
        state->items = field;
        parser_match(parser, ELX_TOKEN_COMMA);
        parser_match(parser, ELX_TOKEN_SEMICOLON);
    }
    parser_expect(parser, ELX_TOKEN_RBRACE, 228, "expected '}' to end state");
    state->end = parser->current.location;
    return state;
}

static elx_ast_t *parse_permissions(elx_parser_t *parser) {
    elx_ast_t *permissions;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    permissions = elx_ast_new(ELX_AST_PERMISSIONS, token.location);
    if (!permissions) return NULL;
    parser_expect(parser, ELX_TOKEN_LBRACE, 229, "expected '{' to start permissions block");
    while (!parser_check(parser, ELX_TOKEN_RBRACE) && parser->current.type != ELX_TOKEN_EOF) {
        elx_ast_t *property;
        if (parser->current.type != ELX_TOKEN_IDENT) {
            parser_error(parser, 230, "expected permission property name", parser->current.location);
            parser_advance(parser);
            continue;
        }
        property = elx_ast_new(ELX_AST_UI_PROPERTY, parser->current.location);
        property->name = copy_string(parser->current.text, parser->current.length);
        parser_advance(parser);
        parser_expect(parser, ELX_TOKEN_COLON, 231, "expected ':' after permission property name");
        property->value = parse_expression(parser, 0);
        property->end = property->value ? property->value->end : parser->current.location;
        property->next = permissions->properties;
        permissions->properties = property;
        parser_match(parser, ELX_TOKEN_COMMA);
        parser_match(parser, ELX_TOKEN_SEMICOLON);
    }
    parser_expect(parser, ELX_TOKEN_RBRACE, 232, "expected '}' to end permissions block");
    permissions->end = parser->current.location;
    return permissions;
}

static elx_ast_t *parse_import(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_IMPORT, token.location);
    if (!node) return NULL;
    if (parser_check(parser, ELX_TOKEN_STRING)) {
        token = parser->current;
        node->name = copy_string(token.string ? token.string : "", token.string ? strlen(token.string) : 0U);
        node->import_path = copy_string(token.string ? token.string : "", token.string ? strlen(token.string) : 0U);
        parser_advance(parser);
    } else if (parser_check(parser, ELX_TOKEN_IDENT)) {
        token = parser->current;
        node->name = copy_string(token.text, token.length);
        node->import_path = copy_string(token.text, token.length);
        parser_advance(parser);
        if (parser->current.type == ELX_TOKEN_DOT) {
            parser_advance(parser);
            if (parser_check(parser, ELX_TOKEN_IDENT)) {
                token = parser->current;
                char *old_path = node->import_path;
                size_t new_len = strlen(old_path ? old_path : "") + 1 + token.length + 1;
                char *new_path = (char *)calloc(new_len, 1);
                if (new_path) {
                    if (old_path) strcat(new_path, old_path);
                    strcat(new_path, ".");
                    strncat(new_path, token.text, token.length);
                    free(old_path);
                    node->import_path = new_path;
                }
                parser_advance(parser);
            }
        }
    } else {
        parser_error(parser, 234, "expected string or identifier after import", parser->current.location);
        parser_advance(parser);
        return node;
    }
    if (parser_match(parser, ELX_TOKEN_AS)) {
        if (parser_check(parser, ELX_TOKEN_IDENT)) {
            token = parser->current;
            node->alias = copy_string(token.text, token.length);
            parser_advance(parser);
        } else {
            parser_error(parser, 235, "expected identifier after 'as'", parser->current.location);
            parser_advance(parser);
        }
    }
    parser_match(parser, ELX_TOKEN_SEMICOLON);
    parser_match(parser, ELX_TOKEN_COMMA);
    return node;
}

static elx_ast_t *parse_export(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_EXPORT, token.location);
    if (!node) return NULL;
    node->is_public = true;
    switch (parser->current.type) {
        case ELX_TOKEN_FN: {
            elx_ast_t *decl = parse_function(parser);
            if (decl) {
                decl->public_value = true;
                node->value = decl;
                if (decl->name) node->name = copy_string(decl->name, strlen(decl->name));
            }
            break;
        }
        case ELX_TOKEN_CONST:
        case ELX_TOKEN_VAR:
        case ELX_TOKEN_LET: {
            token = parser->current;
            parser_advance(parser);
            node->kind = token.type == ELX_TOKEN_CONST ? ELX_AST_CONST : ELX_AST_VAR;
            if (parser_check(parser, ELX_TOKEN_IDENT)) {
                token = parser->current;
                node->name = copy_string(token.text, token.length);
                parser_advance(parser);
            }
            if (parser_match(parser, ELX_TOKEN_COLON)) {
                node->type = parse_type(parser);
            }
            if (parser_match(parser, ELX_TOKEN_ASSIGN)) {
                node->value = parse_expression(parser, 0);
            }
            node->public_value = true;
            parser_match(parser, ELX_TOKEN_SEMICOLON);
            break;
        }
        case ELX_TOKEN_STATE: {
            elx_ast_t *decl = parse_state(parser);
            if (decl) {
                node->value = decl;
                if (decl->name) node->name = copy_string(decl->name, strlen(decl->name));
            }
            break;
        }
        case ELX_TOKEN_WIDGET: {
            if (parser_peek(parser).type == ELX_TOKEN_LBRACE) {
                elx_ast_t *decl = parse_widget(parser);
                if (decl) {
                    node->value = decl;
                    if (decl->name) node->name = copy_string(decl->name, strlen(decl->name));
                }
            } else {
                token = parser->current;
                parser_advance(parser);
                node->kind = ELX_AST_EXPR_STMT;
                if (parser_check(parser, ELX_TOKEN_IDENT)) {
                    token = parser->current;
                    node->name = copy_string(token.text, token.length);
                    parser_advance(parser);
                }
                parser_match(parser, ELX_TOKEN_SEMICOLON);
            }
            break;
        }
        case ELX_TOKEN_STRUCT:
        case ELX_TOKEN_ENUM:
        case ELX_TOKEN_NATIVE:
        case ELX_TOKEN_FFI:
        case ELX_TOKEN_SERVICE:
        case ELX_TOKEN_TASK:
        case ELX_TOKEN_TYPE: {
            token = parser->current;
            parser_advance(parser);
            if (parser_check(parser, ELX_TOKEN_IDENT)) {
                token = parser->current;
                node->name = copy_string(token.text, token.length);
                parser_advance(parser);
            }
            parser_match(parser, ELX_TOKEN_SEMICOLON);
            break;
        }
        default: {
            elx_ast_t *expr = parse_expression(parser, 0);
            if (expr) {
                node->value = expr;
                node->kind = expr->kind;
                if (expr->kind == ELX_AST_IDENT && expr->name) {
                    node->name = copy_string(expr->name, strlen(expr->name));
                }
            }
            break;
        }
    }
    parser_match(parser, ELX_TOKEN_SEMICOLON);
    parser_match(parser, ELX_TOKEN_COMMA);
    return node;
}

static elx_ast_t *parse_module_decl(elx_parser_t *parser) {
    elx_ast_t *node;
    elx_token_t token;
    token = parser->current;
    parser_advance(parser);
    node = elx_ast_new(ELX_AST_MODULE, token.location);
    if (!node) return NULL;
    if (parser_check(parser, ELX_TOKEN_IDENT)) {
        token = parser->current;
        node->module_name = copy_string(token.text, token.length);
        node->name = copy_string(token.text, token.length);
        parser_advance(parser);
    } else {
        parser_error(parser, 236, "expected module name", parser->current.location);
        parser_advance(parser);
    }
    parser_match(parser, ELX_TOKEN_SEMICOLON);
    parser_match(parser, ELX_TOKEN_COMMA);
    return node;
}

static elx_ast_t *parse_declaration(elx_parser_t *parser) {
    switch (parser->current.type) {
        case ELX_TOKEN_FN:
            return parse_function(parser);
        case ELX_TOKEN_STATE:
            return parse_state(parser);
        case ELX_TOKEN_PERMISSIONS:
            return parse_permissions(parser);
        case ELX_TOKEN_IMPORT:
            return parse_import(parser);
        case ELX_TOKEN_EXPORT:
            return parse_export(parser);
        case ELX_TOKEN_MODULE:
            return parse_module_decl(parser);
        case ELX_TOKEN_IDENT:
            if (parser_peek(parser).type == ELX_TOKEN_LBRACE) {
                return parse_widget(parser);
            }
            parser_error(parser, 233, "unknown or unsupported declaration", parser->current.location);
            parser_advance(parser);
            return NULL;
        case ELX_TOKEN_STRUCT:
        case ELX_TOKEN_ENUM:
        case ELX_TOKEN_CONST:
        case ELX_TOKEN_VAR:
        case ELX_TOKEN_LET:
        case ELX_TOKEN_USE:
        case ELX_TOKEN_PUB:
        case ELX_TOKEN_PRIV:
        case ELX_TOKEN_NATIVE:
        case ELX_TOKEN_FFI:
        case ELX_TOKEN_SERVICE:
        case ELX_TOKEN_TASK:
        case ELX_TOKEN_WIDGET:
        case ELX_TOKEN_CAPABILITY:
        case ELX_TOKEN_TYPE:
        default:
            parser_error(parser, 233, "unknown or unsupported declaration", parser->current.location);
            parser_advance(parser);
            return NULL;
    }
}

static elx_ast_t *parse_program(elx_parser_t *parser) {
    elx_ast_t *program;
    elx_ast_t *declaration;
    program = elx_ast_new(ELX_AST_PROGRAM, parser->current.location);
    if (!program) return NULL;
    while (parser->current.type != ELX_TOKEN_EOF) {
        declaration = parse_declaration(parser);
        if (declaration) {
            declaration->next = program->next;
            program->next = declaration;
        }
    }
    program->end = parser->current.location;
    {
        elx_ast_t *prev = NULL;
        elx_ast_t *curr = program->next;
        elx_ast_t *next;
        while (curr) {
            next = curr->next;
            curr->next = prev;
            prev = curr;
            curr = next;
        }
        program->next = prev;
    }
    return program;
}

elx_ast_t *elx_parse(const char *source, size_t length, elx_diagnostic_list_t *diagnostics) {
    elx_parser_t parser;
    elx_ast_t *program;
    if (!source || !diagnostics) return NULL;
    parser.lexer = elx_lexer_create(source, length, diagnostics);
    if (!parser.lexer) return NULL;
    parser.diagnostics = diagnostics;
    parser.have_peek = false;
    parser.current = elx_lexer_next(parser.lexer);
    program = parse_program(&parser);
    elx_token_destroy(&parser.current);
    if (parser.have_peek) elx_token_destroy(&parser.peek);
    elx_lexer_destroy(parser.lexer);
    return program;
}
