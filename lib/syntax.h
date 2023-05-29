#ifndef SYNTAX_H
#define SYNTAX_H

typedef enum {
    UNARY,
    BINARY,
} ExpType;

typedef enum {
    IF,
    VAR,
    MATCH,
    FUN,
} StmtType;

typedef enum {
    PLUS,
    MINUS,
    STAR,
    SLASH,
    AND,
    OR,
} Operator;

/**
 * This type act as a wrapper around all the possible expression type
 * @note: proprer casting is required
 */
typedef struct {
    ExpType type;
    void *exp;
} Exp_t;

typedef struct {
    void *exp;
    Operator op;
} Exp_unary_t;

typedef struct {
    void *left;
    Operator op;
    void *right;
} Exp_binary_t;

typedef struct {
    void * value;
} Exp_literal_t;


typedef struct {
    void *stmt;
    StmtType type;
} Stmt_t;

typedef struct {
    Exp_t *condition;
    Exp_t *then_brench;
    Exp_t *else_brench;
} Stmt_conditional_t;

typedef struct {

} Stmt_match_t;

typedef struct {
    Exp_t exp;
} Stmt_let_t;

typedef struct {

} Stmt_fun_t;

#endif // !SYNTAX_H
