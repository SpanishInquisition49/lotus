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
    EXP_IDENTIFIER,
    EXP_CALL,
} ExpType;

typedef enum {
    T_STRING,
    T_NUMBER,
    T_BOOLEAN,
    T_NIL,
    T_CLOSURE,
} LiteralType;

typedef enum {
    STMT_IF,
    STMT_MATCH,
    STMT_FUN,
    STMT_PRINT,
    STMT_EXPR,
    STMT_BLOCK,
    STMT_DECLARATION,
    STMT_ASSIGNMENT,
} StmtType;

typedef enum {
    OP_ERROR,
    OP_PLUS,
    OP_MINUS,
    OP_STAR,
    OP_MOD,
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
Exp_t *exp_dup(Exp_t*);
void exp_destroy(Exp_t*);
void exp_free(void*);
void *exp_unwrap(Exp_t*);

typedef struct {
    Operator op;
    Exp_t *right;
} Exp_unary_t;

Exp_unary_t *exp_unary_init(Operator, Exp_t*);
Exp_unary_t *exp_unary_dup(Exp_unary_t*);
void exp_unary_destroy(Exp_unary_t*);

typedef struct {
    Exp_t *left;
    Operator op;
    Exp_t *right;
} Exp_binary_t;

Exp_binary_t *exp_binary_init(Exp_t*, Operator, Exp_t*);
Exp_binary_t *exp_binary_dup(Exp_binary_t*);
void exp_binary_destroy(Exp_binary_t*);

typedef struct {
    Exp_t *exp;
} Exp_grouping_t;

Exp_grouping_t *exp_grouping_init(Exp_t*);
Exp_grouping_t *exp_grouping_dup(Exp_grouping_t*);
void exp_groping_destroy(Exp_grouping_t*);

typedef struct {
    void *value;
    LiteralType type;
} Exp_literal_t;

Exp_literal_t *exp_literal_init(LiteralType, void*);
Exp_literal_t *exp_literal_dup(Exp_literal_t*);
void exp_literal_destroy(Exp_literal_t*);

typedef struct {
    char *identifier;
} Exp_identifier_t;

Exp_identifier_t *exp_identifier_init(char*);
Exp_identifier_t *exp_identifier_dup(Exp_identifier_t*);
void exp_identifer_destroy(Exp_identifier_t*);

typedef struct {
    char *identifier;
    l_list_t actuals;
} Exp_call_t;

Exp_call_t *exp_call_init(char*, l_list_t);
Exp_call_t *exp_call_dup(Exp_call_t*);
void exp_call_destroy(Exp_call_t*);

/********************************************************************
 *                          Statements                              *
 ********************************************************************/

typedef struct {
    void *stmt;
    StmtType type;
    int line;
} Stmt_t;

Stmt_t *stmt_init(StmtType, void*, int);
Stmt_t *stmt_dup(Stmt_t*);
void stmt_destroy(Stmt_t*);
void stmt_free(void*);
void *stmt_unwrap(Stmt_t*);

typedef struct {
    Exp_t *condition;
    Stmt_t *then_brench;
    Stmt_t *else_brench;
} Stmt_conditional_t;

Stmt_conditional_t *stmt_conditional_init(Exp_t*, Stmt_t*, Stmt_t*);
Stmt_conditional_t *stmt_conditional_dup(Stmt_conditional_t*);
void stmt_conditional_destroy(Stmt_conditional_t*);

typedef struct {
    Exp_t *exp;
} Stmt_print_t;

Stmt_print_t *stmt_print_init(Exp_t*);
Stmt_print_t *stmt_print_dup(Stmt_print_t*);
void stmt_print_destroy(Stmt_print_t*);

typedef struct {
    Exp_t *exp;
} Stmt_expr_t;

Stmt_expr_t *stmt_expr_init(Exp_t*);
Stmt_expr_t *stmt_expr_dup(Stmt_expr_t*);
void stmt_expr_destroy(Stmt_expr_t*);

typedef struct {
    l_list_t statements;
} Stmt_block_t;

Stmt_block_t *stmt_block_init(l_list_t);
Stmt_block_t *stmt_block_dup(Stmt_block_t*);
void stmt_block_destroy(Stmt_block_t*);

typedef struct {
   char *identifier;
   Exp_t *exp;
} Stmt_declaration_t;

Stmt_declaration_t *stmt_declaration_init(char *, Exp_t*);
Stmt_declaration_t *stmt_declaration_dup(Stmt_declaration_t*);
void stmt_declaration_destroy(Stmt_declaration_t*);

typedef Stmt_declaration_t Stmt_assignment_t;

Stmt_assignment_t *stmt_assignment_init(char*, Exp_t*);
Stmt_assignment_t *stmt_assignment_dup(Stmt_assignment_t*);
void stmt_assignment_destroy(Stmt_assignment_t*);

typedef struct {
    char *identifier;
    l_list_t formals;
    Stmt_t *body;
} Stmt_function_t;

Stmt_function_t *stmt_function_init(char*, l_list_t, Stmt_t*);
Stmt_function_t *stmt_function_dup(Stmt_function_t*);
void stmt_function_destroy(Stmt_function_t*);

Operator token_to_operator(Token);

typedef struct {
    char *identifier;
    l_list_t formals;
    Stmt_t *body;
} closure_t;

#endif // !SYNTAX_H
