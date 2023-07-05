#include "parser.h"
#include "memory.h"
#include "token.h"
#include "syntax.h"
#include "list.h"
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
static void parser_log(Parser*);
static int match(Parser*, int, ...);
static int check(Parser*, TokenType);
static int is_at_end(Parser*);
static Token *previous(Parser*);
static Token *peek(Parser*);
static Token *advance(Parser*);
static Token *consume(Parser*, TokenType, char *);

static Stmt_t *statement(Parser*);
static Stmt_t *stmt_expr(Parser*);
static Stmt_t *stmt_print(Parser*);
static Stmt_t *stmt_condition(Parser*);
static Stmt_t *stmt_block(Parser*);
static Stmt_t *stmt_declaration(Parser*);
static Stmt_t *stmt_assignement(Parser*);

void parser_init(Parser *parser, List tokens) {
    parser->current = 0;
    parser->tokens = tokens;
    for(int i=0; i<LOG_LEVELS; i++)
        parser->errors[i] = 0;
    return;
}

List parser_parse(Parser* parser) {
    List statements = NULL;
    while(!is_at_end(parser)) {
        list_add(&statements, statement(parser));
    }
    list_reverse_in_place(&statements);
    return statements;
}

void parser_errors_report(Parser parser) {
    dprintf(2, "%s[PARSER]\t%sErrors: %d\t%sWarnings: %d%s\n", 
            ANSI_COLOR_MAGENTA,
            ANSI_COLOR_RED,
            parser.errors[ERROR],
            ANSI_COLOR_YELLOW,
            parser.errors[WARNING],
            ANSI_COLOR_RESET
        );
    return;
}

int parser_had_errors(Parser p) {
    return p.errors[ERROR] > 0;
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
    while(match(p, 2, BANG_EQUAL, EQUAL_EQUAL)) {
        Token *prev = previous(p);
        Exp_t *right = comparison(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
        parser_log(p);

    }
    return expr;
}

Exp_t *comparison(Parser *p) {
    Exp_t *expr = NULL; 
    expr = term(p);
    while(match(p, 4, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
        Token *prev = previous(p);
        Exp_t *right = term(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
        parser_log(p);
    }
    return expr;
}

Exp_t *term(Parser *p) {
    Exp_t *expr = NULL;
    expr = factor(p);
    while(match(p, 2, MINUS, PLUS)) {
        Token *prev = previous(p);
        Exp_t *right = factor(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
        parser_log(p);
    }
    return expr;
}

Exp_t *factor(Parser *p) {
   Exp_t *expr = NULL;
   expr = unary(p);
   while(match(p, 3, SLASH, STAR, MOD)) {
        Token *prev = previous(p);
        Exp_t *right = unary(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
        parser_log(p);
   }
   return expr;
}

Exp_t *unary(Parser* p) {
    if(match(p, 2, BANG, MINUS)) {
        Token *prev = previous(p);
        Exp_t *right = unary(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_unary_t *e = exp_unary_init(op, right);
        parser_log(p);
        return exp_init(EXP_UNARY, e);
    }
    return primary(p);
}

Exp_t *primary(Parser *p) {
    if(match(p, 1, FALSE)) {
        int *value = mem_calloc(1, sizeof(int));
        *value = 0;
        Exp_literal_t *e = exp_literal_init(T_BOOLEAN, value);
        return exp_init(EXP_LITERAL, e);
    }
    if(match(p, 1, TRUE)) {
        int *value = mem_calloc(1, sizeof(int));
        *value = 1;
        Exp_literal_t *e = exp_literal_init(T_BOOLEAN, value);
        return exp_init(EXP_LITERAL, e);
    }
    if(match(p, 1, NIL)) 
        return exp_init(EXP_LITERAL, exp_literal_init(T_NIL, NULL));
    if(match(p, 1, NUMBER))
        return exp_init(EXP_LITERAL, exp_literal_init(T_NUMBER, previous(p)->literal));
    if(match(p, 1, STRING))
        return exp_init(EXP_LITERAL, exp_literal_init(T_STRING, previous(p)->literal));
    if(match(p, 1, IDENTIFIER))
        return exp_init(EXP_IDENTIFIER, exp_identifier_init((char*)previous(p)->literal));
    if(match(p, 1, LEFT_PAREN)) {
        Exp_t *expr = expression(p);
        if(consume(p, RIGHT_PAREN, "Expected ')' after expression.\n") == NULL)
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

int match(Parser *p, int params_number, ...) {
    int hit = 0;
    va_list tokens;
    va_start(tokens, params_number);
    for(int i=0; i<params_number; i++) {
        TokenType token = va_arg(tokens, TokenType);
        if(check(p, token)) {
            advance(p);
            hit = 1;
            break;
        }
    }
    va_end(tokens);
    return hit;
}

int check(Parser *p, TokenType token) {
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

Token *consume(Parser* p, TokenType type, char *msg) {
    if(check(p, type)) return advance(p);
    throw_error(p, msg);
    return peek(p);
}

void throw_error(Parser *p, char *msg) {
    p->errors[ERROR]++;
    Token t = *peek(p);
    if(t.type == END)
        Log(ERROR, "[Line: %d] at end: %s", t.line, msg);
    else
        Log(ERROR, "[Line: %d] at '%s': %s", t.line, t.lexeme, msg);
    return;
}

void parser_log(Parser *p) {
    p->errors[INFO]++;
    Token t = *peek(p);
    Log(INFO, "[Line: %d] Created Exp: '%s'\n", t.line, t.lexeme);
}

int is_at_end(Parser* p) {
    return peek(p)->type == END;
}

Stmt_t *statement(Parser *p) {
    if(match(p, 1, VAR)) return stmt_declaration(p);
    if(match(p, 1, IDENTIFIER)) return stmt_assignement(p);
    if(match(p, 1, LEFT_BRACE)) return stmt_block(p);
    if(match(p, 1, IF)) return stmt_condition(p);
    if(match(p, 1, PRINT)) return stmt_print(p);
    return stmt_expr(p);
}

Stmt_t *stmt_expr(Parser *p) {
    Exp_t *exp = expression(p);
    Token *t = consume(p, SEMICOLON, "Expected ';' after expression\n");
    Stmt_expr_t *s = stmt_expr_init(exp);
    return stmt_init(STMT_EXPR, s, t->line);
}

Stmt_t *stmt_print(Parser *p) {
    Exp_t *value = expression(p);
    Token *t = consume(p, SEMICOLON, "Expected ';' after value\n");
    Stmt_print_t *s = stmt_print_init(value);
    return stmt_init(STMT_PRINT, s, t->line);
}

Stmt_t *stmt_condition(Parser *p) {
    Token *t = consume(p, LEFT_PAREN, "Expected '(' after if\n");
    Exp_t *cond = expression(p);
    t = consume(p, RIGHT_PAREN, "Expected ')' after condition");
    Stmt_t *then_brench = statement(p);
    Stmt_t *else_brench = NULL;
    if(match(p, 1, ELSE))
        else_brench = statement(p);
    Stmt_conditional_t *s = stmt_conditional_init(cond, then_brench, else_brench);
    return stmt_init(STMT_IF, s, t->line);
}

Stmt_t *stmt_block(Parser *p) { 
    List statements = NULL;
    while(!check(p, RIGHT_BRACE) && !is_at_end(p)) {
        list_add(&statements, statement(p));
    }
    Token *t = consume(p, RIGHT_BRACE, "Missing '}' after opening a block\n");
    list_reverse_in_place(&statements);
    Stmt_block_t *s = stmt_block_init(statements);
    return stmt_init(STMT_BLOCK, s, t->line);
}

Stmt_t *stmt_declaration(Parser *p) {
    Token *t = consume(p, IDENTIFIER, "Missing identifier after a declaration\n");
    consume(p, EQUAL, "Missing '=' after declaration\n");
    Exp_t *e = expression(p);
    consume(p, SEMICOLON, "Expected ';' after value\n");
    Stmt_declaration_t *s = stmt_declaration_init(t->literal, e);
    return stmt_init(STMT_DECLARATION, s, t->line);
}

Stmt_t *stmt_assignement(Parser *p) {
    char* ide = previous(p)->literal;
    Token *t = consume(p, EQUAL, "Missing '=' between identifier and expression in assignement\n");
    Exp_t *e = expression(p);
    consume(p, SEMICOLON, "Missing ';' after assignement\n");
    Stmt_assignement_t *s = stmt_assignement_init(ide, e);
    return stmt_init(STMT_ASSIGNMENT, s, t->line);
}

