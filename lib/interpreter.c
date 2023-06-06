#include <string.h>
#include "interpreter.h"
#include "syntax.h"
#include "memory.h"

static void value_destroy(Value*);
static Value *eval(Interpreter*, Exp_t*);
static Value *eval_literal(Interpreter*, Exp_t*);
static Value *eval_grouping(Interpreter*, Exp_t*);
static Value *eval_unary(Interpreter*, Exp_t*);
static Value *eval_binary(Interpreter*, Exp_t*);

static int is_equal(Value*, Value*);


void interpreter_init(Interpreter *interpreter, Exp_t* ast) {
    interpreter->ast = ast;
    interpreter->result = NULL;
    return;
}

void interpreter_destroy(Interpreter interpreter) {
    exp_destroy(interpreter.ast);
    return;
}

void interpreter_eval(Interpreter* interpreter) {
    interpreter->result = eval(interpreter, interpreter->ast);
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
            return eval_literal(i, exp);
    }

    // Unreachable
    return NULL;
}

Value *eval_literal(Interpreter *i, Exp_t *exp) {
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
            *((double*)right->value) = -(*((double*)right->value));
            return right;
        case OP_NOT:
            *((int*)right->value) = !(*(int*)right->value);
            return right;
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
            *((double*)left->value) = *((double*)left->value) - *((double*)right->value);
            return left;
        case OP_STAR:
            *((double*)left->value) = *((double*)left->value) * *((double*)right->value);
            return left;
        case OP_SLASH:
            *((double*)left->value) = *((double*)left->value) / *((double*)right->value);
            return left;
        case OP_PLUS:
            *((double*)left->value) = *((double*)left->value) + *((double*)right->value);
            return left;
        case OP_GREATER: {
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) > *((double*)right->value);
            left = exp_literal_init(T_BOOLEAN, value);
            exp_destroy(unwrapped_exp->left);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_GREATER_EQUAL: {
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) >= *((double*)right->value);
            exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_LESS: {
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) < *((double*)right->value);
            exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_LESS_EQUAL: {
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) <= *((double*)right->value);
            exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_EQUAL: {
            int *value = mem_calloc(1, sizeof(int));
            *value = is_equal(left, right);
            exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_NOT_EQUAL: {
            int *value = mem_calloc(1, sizeof(int));
            *value = !is_equal(left, right);
            exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
    }

    // Unreachable
    return NULL;
}

int is_equal(Value *l, Value *r) {
    // Todo: implement a type system better than this
    if(l->type != r->type) return 0;

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

void value_destroy(Value *v) {
    if(v->type != T_NIL)
        free(v->value);
    free(v);
    return;
}

