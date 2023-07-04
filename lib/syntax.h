#ifndef SYNTAX_H
#define SYNTAX_H
#include "list.h"
#include "token.h"

typedef enum {
    EXP_PANIC_MODE, // used only for the parser panic mode
    EXP_UNARY,
    EXP_BINARY,
    EXP_LITERAL,
    EXP_GROUPING,
} ExpType;

typedef enum {
    T_STRING,
    T_NUMBER,
    T_BOOLEAN,
    T_NIL,
} LiteralType;

typedef enum {
    STMT_IF,
    STMT_VAR,
    STMT_MATCH,
    STMT_FUN,
    STMT_PRINT,
    STMT_EXPR,
} StmtType;

typedef enum {
    OP_ERROR,
    OP_PLUS,
    OP_MINUS,
    OP_STAR,
    OP_SLASH,
    OP_AND,
    OP_OR,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_NOT,
} Operator;

/********************************************************************
 *                          Expressions                             *
 ********************************************************************/

/**
 * This type act as a wrapper around all the possible expression type
 * @note: proper casting is required
 */
typedef struct {
    ExpType type;
    void *exp;
} Exp_t;

Exp_t *exp_init(ExpType, void*);
void exp_destroy(Exp_t*);
void *exp_unwrap(Exp_t*);

typedef struct {
    Operator op;
    Exp_t *right;
} Exp_unary_t;

Exp_unary_t *exp_unary_init(Operator, Exp_t*);
void exp_unary_destroy(Exp_unary_t*);

typedef struct {
    Exp_t *left;
    Operator op;
    Exp_t *right;
} Exp_binary_t;

Exp_binary_t *exp_binary_init(Exp_t*, Operator, Exp_t*);
void exp_binary_destroy(Exp_binary_t*);

typedef struct {
    Exp_t *exp;
} Exp_grouping_t;

Exp_grouping_t *exp_grouping_init(Exp_t*);
void exp_groping_destroy(Exp_grouping_t*);

typedef struct {
    void *value;
    LiteralType type;
} Exp_literal_t;

Exp_literal_t *exp_literal_init(LiteralType, void*);
void exp_literal_destroy(Exp_literal_t*);

/********************************************************************
 *                          Statements                              *
 ********************************************************************/

typedef struct {
    void *stmt;
    StmtType type;
    int line;
} Stmt_t;

Stmt_t *stmt_init(StmtType, void*, int);
void stmt_destroy(Stmt_t*);
void stmt_free(void*);
void *stmt_unwrap(Stmt_t*);

typedef struct {
    Exp_t *condition;
    Stmt_t *then_brench;
    Stmt_t *else_brench;
} Stmt_conditional_t;

Stmt_conditional_t *stmt_conditional_init(Exp_t*, Stmt_t*, Stmt_t*);
void stmt_conditional_destroy(Stmt_conditional_t*);

typedef struct {
    Exp_t *exp;
} Stmt_print_t;

Stmt_print_t *stmt_print_init(Exp_t*);
void stmt_print_destroy(Stmt_print_t*);

typedef struct {
    Exp_t *exp;
} Stmt_expr_t;

Stmt_expr_t *stmt_expr_init(Exp_t*);
void stmt_expr_destroy(Stmt_expr_t*);

/*
typedef struct {
    // TODO: write me
} Stmt_match_t;

typedef struct {
    Exp_t *exp;
    Exp_t *follow_up_exp;
} Stmt_let_t;

typedef struct {
    // TODO: write me
} Stmt_fun_t;
*/

Operator token_to_operator(Token);

#endif // !SYNTAX_H
