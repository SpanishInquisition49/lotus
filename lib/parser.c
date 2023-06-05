#include "parser.h"
#include "memory.h"
#include "token.h"
#include "syntax.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static Exp_t *expression(Parser*);
static Exp_t *equality(Parser*);
static Exp_t *comparison(Parser*);
static Exp_t *term(Parser*);
static Exp_t *factor(Parser*);
static Exp_t *unary(Parser*);
static Exp_t *primary(Parser*);
static void synchronize(Parser*);
static void throw_error(Parser*, char *);
static int match(Parser*, ...);
static int check(Parser*, int);
static int is_at_end(Parser*);
static Token *previous(Parser*);
static Token *peek(Parser*);
static Token *advance(Parser*);
static Token *consume(Parser*, enum TokenType, char *);

void parser_init(Parser *parser, List tokens) {
    parser->current = 0;
    parser->tokens = tokens;
    for(int i=0; i<LOG_LEVELS; i++)
        parser->errors[i] = 0;
    return;
}

Exp_t *parser_generate_ast(Parser* parser) {
    return expression(parser);
}

void parser_errors_report(Parser parser) {
    dprintf(2, "%s[PARSER]\t%sErrors: %d\t%sWarnings: %d%s\n", ANSI_COLOR_MAGENTA, ANSI_COLOR_RED, parser.errors[ERROR], ANSI_COLOR_YELLOW, parser.errors[WARNING], ANSI_COLOR_RESET);
    return;
}

void parser_destroy(Parser parser) {
    list_free(parser.tokens, token_free);
    return;
}

Exp_t *expression(Parser *p) {
    return equality(p);
}

Exp_t *equality(Parser *p) {
    Exp_t *expr = NULL;
    expr = comparison(p);
    while(match(p, BANG_EQUAL, EQUAL_EQUAL)) {
        Token *prev = (previous(p));
        Exp_t *rigth = comparison(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_binary_t *e = exp_binary_init(expr, op, rigth);
        expr = exp_init(EXP_BINARY, e);
        Log(INFO, "Created equality expression\n");

    }
    return expr;
}

Exp_t *comparison(Parser *p) {
    Exp_t *expr = NULL; 
    expr = term(p);
    while(match(p, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
        Token *prev = previous(p);
        Exp_t *rigth = term(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_binary_t *e = exp_binary_init(expr, op, rigth);
        expr = exp_init(EXP_BINARY, e);
        Log(INFO, "Created comparison expression\n");
    }
    return expr;
}

Exp_t *term(Parser *p) {
    Exp_t *expr = NULL;
    expr = factor(p);
    while(match(p, MINUS, PLUS)) {
        Token *prev = previous(p);
        Exp_t *rigth = factor(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_binary_t *e = exp_binary_init(expr, op, rigth);
        expr = exp_init(EXP_BINARY, e);
        Log(INFO, "Created term expression\n");
    }
    return expr;
}

Exp_t *factor(Parser *p) {
   Exp_t *expr = NULL;
   expr = unary(p);
   while(match(p, SLASH, STAR)) {
        Token *prev = previous(p);
        Exp_t *rigth = unary(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_binary_t *e = exp_binary_init(expr, op, rigth);
        expr = exp_init(EXP_BINARY, e);
        Log(INFO, "Created factor expression\n");
   }
   return expr;
}

Exp_t *unary(Parser* p) {
    if(match(p, BANG, MINUS)) {
        Token *prev = previous(p);
        Exp_t *rigth = unary(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_unary_t *e = exp_unary_init(op, rigth);
        Log(INFO, "Created unary expression\n");
        return exp_init(EXP_UNARY, e);
    }
    return primary(p);
}

Exp_t *primary(Parser *p) {
    if(match(p, FALSE)) {
        int *value = mem_calloc(1, sizeof(int));
        *value = 0;
        Exp_literal_t *e = exp_literal_init(T_BOOLEAN, value);
        return exp_init(EXP_LITERAL, e);
    }
    if(match(p, TRUE)) {
        int *value = mem_calloc(1, sizeof(int));
        *value = 1;
        Exp_literal_t *e = exp_literal_init(T_BOOLEAN, value);
        return exp_init(EXP_LITERAL, e);
    }
    if(match(p, NIL)) 
        return exp_init(EXP_LITERAL, exp_literal_init(T_NIL, NULL));
    if(match(p, NUMBER))
        return exp_init(EXP_LITERAL, exp_literal_init(T_NUMBER, strdup(previous(p)->literal)));
    if(match(p, STRING))
        return exp_init(EXP_LITERAL, exp_literal_init(T_STRING, strdup(previous(p)->literal)));

    if(match(p, LEFT_PAREN)) {
        Exp_t *expr = expression(p);
        if(consume(p, RIGHT_PAREN, "Expected ')' after exrpession.\n") == NULL)
            return exp_init(EXP_PANIC_MODE, NULL);
        Exp_grouping_t *e = exp_grouping_init(expr);
        return exp_init(EXP_GROUPING, e);
    }

    throw_error(p, "Expected expression.\n");
    return exp_init(EXP_PANIC_MODE, NULL);
}

void synchronize(Parser *p) {
    advance(p);
    while(!is_at_end(p)) {
        if(previous(p)->type == SEMICOLON) return;
        switch (peek(p)->type) {
            case FUN:
            case VAR:
            case IF:
            case PRINT:
            case RETURN:
                return;
            default:
                break;
        }
        advance(p);
    }
    return;
}

int match(Parser *p, ...) {
    va_list tokens;
    va_start(tokens, p);
    int token = 0;
    while((token = va_arg(tokens, int))) {
        if(check(p, token)) {
            advance(p);
            va_end(tokens);
            return 1;
        }
    }
    va_end(tokens);
    return 0;
}

int check(Parser *p, int token) {
    if(is_at_end(p)) return 0;
    return peek(p)->type == token;
}

Token *advance(Parser *p) {
    if(!is_at_end(p)) p->current++;
    return previous(p);
}

Token *peek(Parser *p) {
    return tokens_get(p->tokens, p->current);
}

Token *previous(Parser *p) {
    return tokens_get(p->tokens, (p->current)-1);
}

Token *consume(Parser* p, enum TokenType type, char *msg) {
    if(check(p, type)) return advance(p);
    throw_error(p, msg) ;
    return NULL;
}

void throw_error(Parser *p, char *msg) {
    p->errors[ERROR]++;
    Token t = *peek(p);
    if(t.type == END)
        Log(ERROR, "%d at end: %s", t.line, msg);
    else
        Log(ERROR, "%d at '%s': %s", t.line, t.lexeme, msg);
    return;
}

int is_at_end(Parser* p) {
    return peek(p)->type == END;
}

