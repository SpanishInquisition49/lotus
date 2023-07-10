#include "parser.h"
#include "errors.h"
#include "memory.h"
#include "token.h"
#include "syntax.h"
#include "list.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static Exp_t *expression(Parser*);
static Exp_t *equality(Parser*);
static Exp_t *boolean_algebra(Parser*);
static Exp_t *comparison(Parser*);
static Exp_t *term(Parser*);
static Exp_t *factor(Parser*);
static Exp_t *unary(Parser*);
static Exp_t *primary(Parser*);
static Exp_t *call(Parser*);
static void synchronize(Parser*);
static void throw_error(Parser*, char *);
static void parser_log(Parser*);
static int match(Parser*, int, ...);
static int check(Parser*, TokenType);
static int is_at_end(Parser*);
static Token *peek_previous(Parser*);
static Token *peek(Parser*);
static Token *peek_next(Parser*);
static Token *advance(Parser*);
static Token *back(Parser*);
static Token *consume(Parser*, TokenType, char *);

static Stmt_t *statement(Parser*);
static Stmt_t *stmt_expr(Parser*);
static Stmt_t *stmt_print(Parser*);
static Stmt_t *stmt_condition(Parser*);
static Stmt_t *stmt_block(Parser*);
static Stmt_t *stmt_declaration(Parser*);
static Stmt_t *stmt_assignment(Parser*);
static Stmt_t *stmt_function_declaration(Parser*);

void parser_init(Parser *parser, l_list_t tokens) {
    parser->current = 0;
    parser->tokens = tokens;
    for(int i=0; i<LOG_LEVELS; i++)
        parser->errors[i] = 0;
    return;
}

l_list_t parser_parse(Parser* parser) {
    l_list_t statements = NULL;
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
    expr = boolean_algebra(p);
    while(match(p, 2, BANG_EQUAL, EQUAL_EQUAL)) {
        Token *prev = peek_previous(p);
        Exp_t *right = boolean_algebra(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
        parser_log(p);

    }
    return expr;
}

Exp_t *boolean_algebra(Parser *p) {
    Exp_t *expr = comparison(p);
    while(match(p, 2, AND, OR)) {
        Token *prev = peek_previous(p);
        Exp_t *right = comparison(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
    }
    return expr;  
}

Exp_t *comparison(Parser *p) {
    Exp_t *expr = NULL; 
    expr = term(p);
    while(match(p, 4, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
        Token *prev = peek_previous(p);
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
        Token *prev = peek_previous(p);
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
        Token *prev = peek_previous(p);
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
        Token *prev = peek_previous(p);
        Exp_t *right = unary(p);
        Operator op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        Exp_unary_t *e = exp_unary_init(op, right);
        parser_log(p);
        return exp_init(EXP_UNARY, e);
    }
    return call(p);
}

Exp_t *call(Parser *p) {
    if(!match(p, 1, IDENTIFIER))
        return primary(p);
    char *ide = peek_previous(p)->literal;
    if(match(p, 1, LEFT_PAREN)) {
        l_list_t actuals = NULL;
        while(!check(p, RIGHT_PAREN) && !is_at_end(p)) {
            Exp_t *actual = expression(p);
            list_add(&actuals, actual);
            if(check(p, RIGHT_PAREN))
                break;
            consume(p, COMMA, "Missing ',' between actuals\n");
        }
        consume(p, RIGHT_PAREN, "Missing ')' after actuals\n");
        Exp_call_t *e = exp_call_init(ide,  actuals);
        return exp_init(EXP_CALL, e);
    }
    back(p);
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
        return exp_init(EXP_LITERAL, exp_literal_init(T_NUMBER, peek_previous(p)->literal));
    if(match(p, 1, STRING))
        return exp_init(EXP_LITERAL, exp_literal_init(T_STRING, peek_previous(p)->literal));
    if(match(p, 1, IDENTIFIER))
        return exp_init(EXP_IDENTIFIER, exp_identifier_init((char*)peek_previous(p)->literal));
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
        if(peek_previous(p)->type == SEMICOLON) return;
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
    return peek_previous(p);
}

Token *back(Parser *p) {
    Token *old = peek(p);
    p->current--;
    return old;

}

Token *peek(Parser *p) {
    return tokens_get(p->tokens, p->current);
}

Token *peek_previous(Parser *p) {
    return tokens_get(p->tokens, (p->current)-1);
}

Token *peek_next(Parser *p) {
    return tokens_get(p->tokens, p->current+1);
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
    if(match(p, 1, FUN)) return stmt_function_declaration(p);
    if(check(p, IDENTIFIER)) return peek_next(p)->type == EQUAL ? stmt_assignment(p) : stmt_expr(p);
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
    l_list_t statements = NULL;
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

Stmt_t *stmt_assignment(Parser *p) {
    char* ide = advance(p)->literal;
    Token *t = consume(p, EQUAL, "Missing '=' between identifier and expression in assignment\n");
    Exp_t *e = expression(p);
    consume(p, SEMICOLON, "Missing ';' after assignment\n");
    Stmt_assignment_t *s = stmt_assignment_init(ide, e);
    return stmt_init(STMT_ASSIGNMENT, s, t->line);
}

Stmt_t *stmt_function_declaration(Parser *p) {
    Token *t = consume(p, IDENTIFIER, "Functions must have a name\n");
    l_list_t formals = NULL;
    consume(p, LEFT_PAREN, "Missing '(' after function name\n");
    while(!check(p, RIGHT_PAREN) && !is_at_end(p)) {
        if(match(p, 1, IDENTIFIER)) {
            list_add(&formals, strdup(peek_previous(p)->literal));
        }
        if(check(p, RIGHT_PAREN))
            break;
        consume(p, COMMA, "Missing ',' between formals\n");
    }
    consume(p, RIGHT_PAREN, "Missing ')' after formals\n");
    Stmt_t *body = statement(p);
    Stmt_function_t *s = stmt_function_init(t->literal, formals, body);
    return stmt_init(STMT_FUN, s, t->line);
}

