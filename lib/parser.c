#include "parser.h"
#include "errors.h"
#include "memory.h"
#include "token.h"
#include "syntax.h"
#include "list.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

/**
 * Parse an expression
 * @param p a pointer to the parser
 * @return a pointer to the parsed expression
 * @note This is the facade for the real expression parsing functions
 */
static exp_t *expression(parser_t*);
/**
 * Parse a forwarding expression
 * @param p a pointer to the parser
 * @return a pointer to the parsed expression
 */
static exp_t *forwarding(parser_t*);
/**
 * Parse an equality expression
 * @param p a pointer to the parser
 * @return a pointer to the parsed expression
 */
static exp_t *equality(parser_t*);
/**
 * Parse an AND/OR expression
 * @param p a pointer to the parser
 * @return a pointer to the parsed expression
 */
static exp_t *boolean_algebra(parser_t*);
/**
 * Parse a comparison expression
 * @param p a pointer to the parser
 * @return a pointer to the parsed expression
 */
static exp_t *comparison(parser_t*);
/**
 * Parse a term
 * @param p a pointer to the parser
 * @return a pointer to the parsed expression
 */
static exp_t *term(parser_t*);
/**
 * Parse a factor
 * @pararm p a pointer to the parser
 * @return a pointer to the parsed expression
 */
static exp_t *factor(parser_t*);
/**
 * Parse an unary expression
 * @param p a pointer to the parser
 * @return a pointer to the parsed expression
 */
static exp_t *unary(parser_t*);
/**
 * Parse a primary expression
 * @param p a pointer to the parser
 * @return a pointer to the parsed expression
 */
static exp_t *primary(parser_t*);
/**
 * Parse a call expression
 * @param p a pointer to the parser
 * @return a pointer to the parsed expression
 */
static exp_t *call(parser_t*);

/**
 * Move the parser to the beginning of the next statement
 * @param p a pointer to the parser
 */
static void synchronize(parser_t*);
/**
 * Stop the parse for the current statement 
 * and print a formatte message to STDERR
 * @param p a pointer to the parser
 * @param fmt the formatted message to be printed
 * @note This function call longjmp, setjmp is called in the statement function
 */
__attribute__((noreturn))
static void throw_error_formatted(parser_t*, char *, va_list);
static void throw_error(parser_t*, char *);
__attribute__((deprecated("This function is no longer needed\n")))
static void parser_log(parser_t*);
/**
 * Check if the current token match one of the given tokens
 * @param p a pointer to the parser
 * @param params_number the number of the token to match
 * @param ... a list of token to match
 * @return 1 if the current token match any of the given tokens, 0 otherwise
 * @note On a successful call the parser automatically advance
 */
static int match(parser_t*, int, ...);
/**
 * Check if the current token match the given token
 * @param p a pointer to the parser
 * @param token the token to match
 * @return 1 if the current token match the given token, 0 otherwise
 * @note The parser do not advance automatically on a successful call
 */
static int check(parser_t*, token_type_t);
/**
 * Check if the parser is a the end of the token list
 * @param p a pointer to the parser
 * @return 1 if the parser is at the end, 0 otherwise
 */
static int is_at_end(parser_t*);
/**
 * Get the token before the current one
 * @param p a pointer to the parser
 * @return a pointer to the previous token
 */
static token_t *peek_previous(parser_t*);
/**
 * Get the current token
 * @param p a pointer to the parser
 * @return a pointer to the current token
 */
static token_t *peek(parser_t*);
/**
 * Get the next token in the list
 * @pararm p a pointer to the parser_destroy
 * @return a pointer to the next token in the list or NULL
 */
static token_t *peek_next(parser_t*);
/**
 * Move to the next token in the list
 * @param p a pointer to the parser
 * @return a pointer to the previous token in the list
 */
static token_t *advance(parser_t*);
/**
 * Move to the previous token in the list
 * @param p a pointer to the token list
 * @return a pointer to the next token in the list
 */
static token_t *back(parser_t*);
/**
 * Check if the current token is the expected one and advance
 * @param p a pointer to the parser
 * @param type the expected token type
 * @param msg an error message to show in case the current token do not match  the given one
 * @return a pointer to the previous token in the list
 * @note In case the token don't match the given one then the synchronize function will be called and the parser proceed to the next statement
 */
__attribute__((format (printf, 3, 4)))
static token_t *consume(parser_t*, token_type_t, char *, ...);

/**
 * Parse a statement
 * @param p a pointer to the parser
 * @return a pointer to the parsed statement
 * @note This is the facade for the real statements parsing functions
 */
static stmt_t *statement(parser_t*);
/**
 * Parse an expression statement
 * @pararm p a pointer to the parser
 * @return a pointer to the parsed statement
 */
static stmt_t *stmt_expr(parser_t*);
/**
 * Parse a print statement
 * @param p a pointer to the parser
 * @return a pointer to the parsed statement
 */
static stmt_t *stmt_print(parser_t*);
/**
 * Parse a conditional statement
 * @param p a pointer to the parser
 * @return a pointer to the parsed statement
 */
static stmt_t *stmt_condition(parser_t*);
/**
 * Parse a block statement
 * @param p a pointer to the parser
 * @return a pointer to the parsed statment
 */
static stmt_t *stmt_block(parser_t*);
/**
 * Parse a declaration statement
 * @param p a pointer to the parser
 * @return a pointer to the parsed statement;
 */
static stmt_t *stmt_declaration(parser_t*);
/**
 * Parse an assignment statement
 * @param p a pointer to the parser
 * @return a pointer to the parsed statement
 */
static stmt_t *stmt_assignment(parser_t*);
/**
 * Parse a function declaration statement
 * @param p a pointer to the parser
 * @return a pointer to the parsed statement
 */
static stmt_t *stmt_function_declaration(parser_t*);
/**
 * Parse a return statement
 * @pararm p a pointer to the parser
 * @return a pointer to the parsed statement
 */
static stmt_t *stmt_return(parser_t*);

void parser_init(parser_t *parser, l_list_t tokens) {
    memset(parser, 0, sizeof(*parser));
    parser->current = 0;
    parser->tokens = tokens;
    return;
}

l_list_t parser_parse(parser_t* parser) {
    l_list_t statements = NULL;
    while(!is_at_end(parser)) {
        list_add(&statements, statement(parser));
    }
    list_reverse_in_place(&statements);
    return statements;
}

void parser_errors_report(parser_t parser) {
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

int parser_had_errors(parser_t p) {
    return p.errors[ERROR] > 0;
}

void parser_destroy(parser_t parser) {
    list_free(parser.tokens, token_free);
    return;
}

exp_t *expression(parser_t *p) {
    return forwarding(p);
}

exp_t *forwarding(parser_t *p) {
    exp_t * expr = equality(p);
    while(match(p, 1, PIPE_GREATER)) {
        exp_t *right = equality(p);
        exp_binary_t *e = exp_binary_init(expr, OP_FORWARD, right);
        expr = exp_init(EXP_BINARY, e);
    }
    return expr;
}

exp_t *equality(parser_t *p) {
    exp_t *expr = NULL;
    expr = boolean_algebra(p);
    while(match(p, 2, BANG_EQUAL, EQUAL_EQUAL)) {
        token_t *prev = peek_previous(p);
        exp_t *right = boolean_algebra(p);
        operator_t op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
        parser_log(p);

    }
    return expr;
}

exp_t *boolean_algebra(parser_t *p) {
    exp_t *expr = comparison(p);
    while(match(p, 2, AND, OR)) {
        token_t *prev = peek_previous(p);
        exp_t *right = comparison(p);
        operator_t op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
    }
    return expr;  
}

exp_t *comparison(parser_t *p) {
    exp_t *expr = NULL; 
    expr = term(p);
    while(match(p, 4, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
        token_t *prev = peek_previous(p);
        exp_t *right = term(p);
        operator_t op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
        parser_log(p);
    }
    return expr;
}

exp_t *term(parser_t *p) {
    exp_t *expr = NULL;
    expr = factor(p);
    while(match(p, 2, MINUS, PLUS)) {
        token_t *prev = peek_previous(p);
        exp_t *right = factor(p);
        operator_t op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
        parser_log(p);
    }
    return expr;
}

exp_t *factor(parser_t *p) {
   exp_t *expr = NULL;
   expr = unary(p);
   while(match(p, 3, SLASH, STAR, MOD)) {
        token_t *prev = peek_previous(p);
        exp_t *right = unary(p);
        operator_t op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        exp_binary_t *e = exp_binary_init(expr, op, right);
        expr = exp_init(EXP_BINARY, e);
        parser_log(p);
   }
   return expr;
}

exp_t *unary(parser_t* p) {
    if(match(p, 2, BANG, MINUS)) {
        token_t *prev = peek_previous(p);
        exp_t *right = unary(p);
        operator_t op = OP_ERROR;
        if(prev)
            op = token_to_operator(*prev);
        exp_unary_t *e = exp_unary_init(op, right);
        parser_log(p);
        return exp_init(EXP_UNARY, e);
    }
    return call(p);
}

exp_t *call(parser_t *p) {
    if(!match(p, 1, IDENTIFIER))
        return primary(p);
    char *ide = peek_previous(p)->literal;
    if(match(p, 1, LEFT_PAREN)) {
        l_list_t actuals = NULL;
        while(!check(p, RIGHT_PAREN) && !is_at_end(p)) {
            exp_t *actual = expression(p);
            list_add(&actuals, actual);
            if(check(p, RIGHT_PAREN))
                break;
            consume(p, COMMA, "Missing ',' between actuals\n");
        }
        consume(p, RIGHT_PAREN, "Missing ')' after actuals\n");
        exp_call_t *e = exp_call_init(ide,  actuals);
        return exp_init(EXP_CALL, e);
    }
    back(p);
    return primary(p);
}

exp_t *primary(parser_t *p) {
    if(match(p, 1, FALSE)) {
        int *value = mem_calloc(1, sizeof(int));
        *value = 0;
        exp_literal_t *e = exp_literal_init(T_BOOLEAN, value);
        return exp_init(EXP_LITERAL, e);
    }
    if(match(p, 1, TRUE)) {
        int *value = mem_calloc(1, sizeof(int));
        *value = 1;
        exp_literal_t *e = exp_literal_init(T_BOOLEAN, value);
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
        exp_t *expr = expression(p);
        if(consume(p, RIGHT_PAREN, "Expected ')' after expression.\n") == NULL)
            return exp_init(EXP_PANIC_MODE, NULL);
        exp_grouping_t *e = exp_grouping_init(expr);
        return exp_init(EXP_GROUPING, e);
    }

    throw_error(p, "Expected expression.\n");
    return exp_init(EXP_PANIC_MODE, NULL);
}

void synchronize(parser_t *p) {
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

int match(parser_t *p, int params_number, ...) {
    int hit = 0;
    va_list tokens;
    va_start(tokens, params_number);
    for(int i=0; i<params_number; i++) {
        token_type_t token = va_arg(tokens, token_type_t);
        if(check(p, token)) {
            advance(p);
            hit = 1;
            break;
        }
    }
    va_end(tokens);
    return hit;
}

int check(parser_t *p, token_type_t token) {
    if(is_at_end(p)) return 0;
    return peek(p)->type == token;
}

token_t *advance(parser_t *p) {
    if(!is_at_end(p)) p->current++;
    return peek_previous(p);
}

token_t *back(parser_t *p) {
    token_t *old = peek(p);
    p->current--;
    return old;

}

token_t *peek(parser_t *p) {
    return tokens_get(p->tokens, p->current);
}

token_t *peek_previous(parser_t *p) {
    return tokens_get(p->tokens, (p->current)-1);
}

token_t *peek_next(parser_t *p) {
    return tokens_get(p->tokens, p->current+1);
}

token_t *consume(parser_t* p, token_type_t type, char *fmt, ...) {
    if(check(p, type)) return advance(p);
    va_list args;
    va_start(args, fmt);
    throw_error_formatted(p, fmt, args);
    va_end(args);
    return peek(p);
}

void throw_error_formatted(parser_t *p, char *msg, va_list args) {
    p->errors[ERROR]++;
    token_t t = *peek(p);
    if(t.type == END)
        err_log(ERROR, "[Line: %d] at end: ", t.line);
    else
        err_log(ERROR, "[Line: %d] at '%s': ", t.line, t.lexeme);
    vdprintf(2, msg, args);
    // This will jump to the statement function and notify that an error as occurred
    longjmp(p->checkpoint, 1);
}

void throw_error(parser_t *p, char *msg) {
    throw_error_formatted(p, msg, NULL);
    return;
}


void parser_log(parser_t *p) {
    p->errors[INFO]++;
    token_t t = *peek(p);
    err_log(INFO, "[Line: %d] Created Exp: '%s'\n", t.line, t.lexeme);
}

int is_at_end(parser_t* p) {
    return peek(p)->type == END;
}

stmt_t *statement(parser_t *p) {
    // This jump act as a try/catch block
    // if an expression throw an error then a synchronize procedure is called 
    // and the parser will continue to parse other statements in order to give an accurate error reports
    int jmp = setjmp(p->checkpoint);
    if(jmp) {
        synchronize(p);
        return NULL;
    }
    if(match(p, 1, VAR)) return stmt_declaration(p);
    if(match(p, 1, FUN)) return stmt_function_declaration(p);
    if(check(p, IDENTIFIER)) return peek_next(p)->type == EQUAL ? stmt_assignment(p) : stmt_expr(p);
    if(match(p, 1, LEFT_BRACE)) return stmt_block(p);
    if(match(p, 1, IF)) return stmt_condition(p);
    if(match(p, 1, PRINT)) return stmt_print(p);
    if(match(p, 1, RETURN)) return stmt_return(p);
    return stmt_expr(p);
}

stmt_t *stmt_expr(parser_t *p) {
    exp_t *exp = expression(p);
    token_t *t = consume(p, SEMICOLON, "Expected ';' after expression\n");
    stmt_expr_t *s = stmt_expr_init(exp);
    return stmt_init(STMT_EXPR, s, t->line);
}

stmt_t *stmt_print(parser_t *p) {
    exp_t *value = expression(p);
    token_t *t = consume(p, SEMICOLON, "Expected ';' after value\n");
    stmt_print_t *s = stmt_print_init(value);
    return stmt_init(STMT_PRINT, s, t->line);
}

stmt_t *stmt_condition(parser_t *p) {
    token_t *t = consume(p, LEFT_PAREN, "Expected '(' after if\n");
    exp_t *cond = expression(p);
    t = consume(p, RIGHT_PAREN, "Expected ')' after condition");
    stmt_t *then_branch = statement(p);
    stmt_t *else_branch = NULL;
    if(match(p, 1, ELSE))
        else_branch = statement(p);
    stmt_conditional_t *s = stmt_conditional_init(cond, then_branch, else_branch);
    return stmt_init(STMT_IF, s, t->line);
}

stmt_t *stmt_block(parser_t *p) { 
    l_list_t statements = NULL;
    while(!check(p, RIGHT_BRACE) && !is_at_end(p)) {
        list_add(&statements, statement(p));
    }
    token_t *t = consume(p, RIGHT_BRACE, "Missing '}' after opening a block\n");
    list_reverse_in_place(&statements);
    stmt_block_t *s = stmt_block_init(statements);
    return stmt_init(STMT_BLOCK, s, t->line);
}

stmt_t *stmt_declaration(parser_t *p) {
    token_t *t = consume(p, IDENTIFIER, "Missing identifier after a declaration\n");
    consume(p, EQUAL, "Missing '=' after declaration\n");
    exp_t *e = expression(p);
    consume(p, SEMICOLON, "Expected ';' after value\n");
    stmt_declaration_t *s = stmt_declaration_init(t->literal, e);
    return stmt_init(STMT_DECLARATION, s, t->line);
}

stmt_t *stmt_assignment(parser_t *p) {
    char* ide = advance(p)->literal;
    token_t *t = consume(p, EQUAL, "Missing '=' between identifier and expression in assignment\n");
    exp_t *e = expression(p);
    consume(p, SEMICOLON, "Missing ';' after assignment\n");
    stmt_assignment_t *s = stmt_assignment_init(ide, e);
    return stmt_init(STMT_ASSIGNMENT, s, t->line);
}

stmt_t *stmt_function_declaration(parser_t *p) {
    token_t *t = consume(p, IDENTIFIER, "Functions must have a name\n");
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
    stmt_t *body = statement(p);
    stmt_function_t *s = stmt_function_init(t->literal, formals, body);
    return stmt_init(STMT_FUN, s, t->line);
}

stmt_t *stmt_return(parser_t *p) {
    exp_t *exp = expression(p);
    stmt_expr_t *s = stmt_expr_init(exp);
    token_t *t = consume(p, SEMICOLON, "Missing ';' after return\n");
    return stmt_init(STMT_RETURN, s, t->line);    
}
