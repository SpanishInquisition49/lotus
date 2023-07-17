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
} exp_type_t;

typedef enum {
    T_STRING,
    T_NUMBER,
    T_BOOLEAN,
    T_NIL,
    T_CLOSURE,
} literal_type_t;

char* literal_type_to_string(literal_type_t);

typedef enum {
    STMT_IF,
    STMT_MATCH,
    STMT_FUN,
    STMT_PRINT,
    STMT_EXPR,
    STMT_BLOCK,
    STMT_DECLARATION,
    STMT_ASSIGNMENT,
    STMT_RETURN,
} stmt_type_t;

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
    OP_FORWARD,
} operator_t;

/********************************************************************
 *                          Expressions                             *
 ********************************************************************/

/**
 * This type act as a wrapper around all the possible expression type
 * @note: proper casting is required
 */
typedef struct {
    exp_type_t type;
    void *exp;
} exp_t;

exp_t *exp_init(exp_type_t, void*);
exp_t *exp_dup(exp_t*);
void exp_destroy(exp_t*);
void exp_free(void*);
void *exp_unwrap(exp_t*);

typedef struct {
    operator_t op;
    exp_t *right;
} exp_unary_t;

exp_unary_t *exp_unary_init(operator_t, exp_t*);
exp_unary_t *exp_unary_dup(exp_unary_t*);
void exp_unary_destroy(exp_unary_t*);

typedef struct {
    exp_t *left;
    operator_t op;
    exp_t *right;
} exp_binary_t;

exp_binary_t *exp_binary_init(exp_t*, operator_t, exp_t*);
exp_binary_t *exp_binary_dup(exp_binary_t*);
void exp_binary_destroy(exp_binary_t*);

typedef struct {
    exp_t *exp;
} exp_grouping_t;

exp_grouping_t *exp_grouping_init(exp_t*);
exp_grouping_t *exp_grouping_dup(exp_grouping_t*);
void exp_groping_destroy(exp_grouping_t*);

typedef struct {
    void *value;
    literal_type_t type;
} exp_literal_t;

exp_literal_t *exp_literal_init(literal_type_t, void*);
exp_literal_t *exp_literal_dup(exp_literal_t*);
void exp_literal_destroy(exp_literal_t*);

typedef struct {
    char *identifier;
} exp_identifier_t;

exp_identifier_t *exp_identifier_init(char*);
exp_identifier_t *exp_identifier_dup(exp_identifier_t*);
void exp_identifier_destroy(exp_identifier_t*);

typedef struct {
    char *identifier;
    l_list_t actuals;
} exp_call_t;

exp_call_t *exp_call_init(char*, l_list_t);
exp_call_t *exp_call_dup(exp_call_t*);
void exp_call_destroy(exp_call_t*);

/********************************************************************
 *                          Statements                              *
 ********************************************************************/

typedef struct {
    void *stmt;
    stmt_type_t type;
    int line;
} stmt_t;

stmt_t *stmt_init(stmt_type_t, void*, int);
stmt_t *stmt_dup(stmt_t*);
void stmt_destroy(stmt_t*);
void stmt_free(void*);
void *stmt_unwrap(stmt_t*);

typedef struct {
    exp_t *condition;
    stmt_t *then_branch;
    stmt_t *else_branch;
} stmt_conditional_t;

stmt_conditional_t *stmt_conditional_init(exp_t*, stmt_t*, stmt_t*);
stmt_conditional_t *stmt_conditional_dup(stmt_conditional_t*);
void stmt_conditional_destroy(stmt_conditional_t*);

typedef struct {
    exp_t *exp;
} stmt_print_t;

stmt_print_t *stmt_print_init(exp_t*);
stmt_print_t *stmt_print_dup(stmt_print_t*);
void stmt_print_destroy(stmt_print_t*);

typedef struct {
    exp_t *exp;
} stmt_expr_t;

stmt_expr_t *stmt_expr_init(exp_t*);
stmt_expr_t *stmt_expr_dup(stmt_expr_t*);
void stmt_expr_destroy(stmt_expr_t*);

typedef struct {
    l_list_t statements;
} stmt_block_t;

stmt_block_t *stmt_block_init(l_list_t);
stmt_block_t *stmt_block_dup(stmt_block_t*);
void stmt_block_destroy(stmt_block_t*);

typedef struct {
   char *identifier;
   exp_t *exp;
} stmt_declaration_t;

stmt_declaration_t *stmt_declaration_init(char *, exp_t*);
stmt_declaration_t *stmt_declaration_dup(stmt_declaration_t*);
void stmt_declaration_destroy(stmt_declaration_t*);

typedef stmt_declaration_t stmt_assignment_t;

stmt_assignment_t *stmt_assignment_init(char*, exp_t*);
stmt_assignment_t *stmt_assignment_dup(stmt_assignment_t*);
void stmt_assignment_destroy(stmt_assignment_t*);

typedef struct {
    char *identifier;
    l_list_t formals;
    stmt_t *body;
} stmt_function_t;

stmt_function_t *stmt_function_init(char*, l_list_t, stmt_t*);
stmt_function_t *stmt_function_dup(stmt_function_t*);
void stmt_function_destroy(stmt_function_t*);

operator_t token_to_operator(token_t);

typedef struct {
    char *identifier;
    l_list_t formals;
    stmt_t *body;
} closure_t;

#endif // !SYNTAX_H
