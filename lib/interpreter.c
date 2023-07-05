#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "interpreter.h"
#include "list.h"
#include "syntax.h"
#include "memory.h"
#include "errors.h"

static Value *eval(Interpreter*, Exp_t*);
static Value *eval_literal(Exp_t*);
static Value *eval_grouping(Interpreter*, Exp_t*);
static Value *eval_unary(Interpreter*, Exp_t*);
static Value *eval_binary(Interpreter*, Exp_t*);

static void eval_stmt(Interpreter*, Stmt_t*);
static void eval_stmt_exp(Interpreter*, Stmt_t*);
static void eval_stmt_print(Interpreter*, Stmt_t*);
static void eval_stmt_conditional(Interpreter*, Stmt_t*);
static void eval_stmt_block(Interpreter*, Stmt_t* );

static int is_equal(Interpreter*, Value*, Value*);
static int is_truthy(Interpreter*, Value*);
static void pretty_print(Value*);

static void raise_runtime_error(Interpreter*, char*);

void interpreter_init(Interpreter *interpreter, List statements) {
    interpreter->statements = statements;
    return;
}

void interpreter_destroy(Interpreter interpreter) {
    list_free(interpreter.statements, stmt_free);
    return;
}

void interpreter_eval(Interpreter* interpreter) {
    List stmts = interpreter->statements;
    while(stmts) {
        eval_stmt(interpreter, stmts->data);
        stmts = stmts->next;
    }
    return;
}

void eval_stmt(Interpreter *i, Stmt_t* s) {
    switch (s->type) {
        case STMT_EXPR:
            eval_stmt_exp(i, s);
            break;
        case STMT_PRINT:
            eval_stmt_print(i, s);
            break;
        case STMT_IF:
            eval_stmt_conditional(i, s);
            break;
        case STMT_BLOCK:
            eval_stmt_block(i, s);
            break;
        default:
            raise_runtime_error(i, "Unimplemented Error\n");
            break;
    }
    return;
}

Value *eval(Interpreter* i, Exp_t *exp) {
    switch(exp->type) {
        case EXP_UNARY:
            return eval_unary(i, exp);
        case EXP_BINARY:
            return eval_binary(i, exp);
        case EXP_GROUPING:
            return eval_grouping(i, exp);
        case EXP_LITERAL:
            return eval_literal(exp);
        default: // Unreachable
            raise_runtime_error(i, "Unknown expression\n");
    }

    // Unreachable
    return NULL;
}

Value *eval_literal(Exp_t *exp) {
    return (Value*)exp->exp;
}

Value *eval_grouping(Interpreter* i, Exp_t *exp) {
    Exp_grouping_t *unwrapped_exp = exp_unwrap(exp);
    return eval(i, unwrapped_exp->exp);
}

Value *eval_unary(Interpreter *i, Exp_t *exp) {
    Exp_unary_t *unwrapped_exp = exp_unwrap(exp);
    Value *right = eval(i, unwrapped_exp->right);

    switch(unwrapped_exp->op) {
        case OP_MINUS:
            if(right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operand must be a number\n");
            *((double*)right->value) = -(*((double*)right->value));
            return right;
        case OP_NOT: {
            int *value = mem_calloc(1, sizeof(int));
            *value = !is_truthy(i, right);
            exp_destroy(unwrapped_exp->right);
            right = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->right = exp_init(EXP_LITERAL, right);
            return right;
        }
        default: // Theoretically unreachable
            raise_runtime_error(i, "Unkown operation\n");
    }

    // Unreachable
    return NULL;
}

Value *eval_binary(Interpreter *i, Exp_t *exp) {
    Exp_binary_t *unwrapped_exp = exp_unwrap(exp);
    Value *right = eval(i, unwrapped_exp->right);
    Value *left = eval(i, unwrapped_exp->left);

    switch (unwrapped_exp->op) {
        case OP_MINUS:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            *((double*)left->value) = *((double*)left->value) - *((double*)right->value);
            return left;
        case OP_STAR:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            *((double*)left->value) = *((double*)left->value) * *((double*)right->value);
            return left;
        case OP_SLASH:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            *((double*)left->value) = *((double*)left->value) / *((double*)right->value);
            return left;
        case OP_PLUS:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            *((double*)left->value) = *((double*)left->value) + *((double*)right->value);
            return left;
        case OP_MOD:
            if(left->type != T_NUMBER || right->type != T_NUMBER){
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            }
            *((double*)left->value) = fmod(*((double*)left->value), *((double*)right->value));
            return left;
        case OP_GREATER: {
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) > *((double*)right->value);
            left = exp_literal_init(T_BOOLEAN, value);
            exp_destroy(unwrapped_exp->left);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_GREATER_EQUAL: {
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) >= *((double*)right->value);
            exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_LESS: {
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) < *((double*)right->value);
            exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_LESS_EQUAL: {
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) <= *((double*)right->value);
            exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_EQUAL: {
            int *value = mem_calloc(1, sizeof(int));
            *value = is_equal(i, left, right);
            exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_NOT_EQUAL: {
            int *value = mem_calloc(1, sizeof(int));
            *value = !is_equal(i, left, right);
            exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        default: // Theoretically unreachable
            raise_runtime_error(i, "Unkown Operation\n");
    }

    // Unreachable
    return NULL;
}

void eval_stmt_exp(Interpreter *i, Stmt_t *s) {
    Stmt_expr_t *unwrapped_stmt = stmt_unwrap(s);
    eval(i, unwrapped_stmt->exp);
    return;
}

void eval_stmt_print(Interpreter *i, Stmt_t *s) {
    Stmt_print_t *unwrapped_stmt = stmt_unwrap(s);
    Value *v = eval(i, unwrapped_stmt->exp);
    pretty_print(v);
    return; 
}

void eval_stmt_conditional(Interpreter *i, Stmt_t *s) {
    Stmt_conditional_t *unwrapped_stmt = stmt_unwrap(s);
    Value *cond = eval(i, unwrapped_stmt->condition);
    if(is_truthy(i, cond))
        eval_stmt(i, unwrapped_stmt->then_brench);
    else if(unwrapped_stmt->else_brench != NULL)
        eval_stmt(i, unwrapped_stmt->else_brench);
    return;
}

void eval_stmt_block(Interpreter *i, Stmt_t *s) {
    Stmt_block_t *unwrapped_stmt = stmt_unwrap(s);
    List stmts = unwrapped_stmt->statements;
    while(stmts) {
        eval_stmt(i, stmts->data);
        stmts = stmts->next;
    }
    return;
}

int is_equal(Interpreter *i, Value *l, Value *r) {
    if(l->type != r->type)
        raise_runtime_error(i, "Type Error:\tComparison between 2 different type!\n");

    switch(l->type) {
        case T_NUMBER:
            return *(double*)l->value == *(double*)r->value;
        case T_NIL:
            return 1;
        case T_BOOLEAN:
            return *(int*)l->value == *(int*)r->value;
        case T_STRING:
            return strcmp((char*)l->value, (char*)r->value) == 0;
    }
    return 0;
}

int is_truthy(Interpreter *i, Value *v) {
    switch(v->type) {
        case T_BOOLEAN:
            return *((int*)v->value);
        default:
            raise_runtime_error(i, "Type Error:\tImplicit casting is not permitted!\n");
            // Unreachable
            return 0;
    }
}

void pretty_print(Value *v) {
    switch (v->type) {
        case T_NUMBER:
            printf("%f\n", *((double*)v->value));
            break;
        case T_STRING:
            printf("%s\n", (char*)v->value);
            break;
        case T_NIL:
            printf("nil\n");
            break;
        case T_BOOLEAN:
            printf("%s\n", *((int*)v->value) ? "true" : "false");
    }
    fflush(stdout);
    return;
}

void raise_runtime_error(Interpreter *i, char *msg) {
    Log(ERROR, msg);
    interpreter_destroy(*i);
    exit(EXIT_FAILURE);
}

