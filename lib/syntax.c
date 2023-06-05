#include "syntax.h"
#include "memory.h"
#include "token.h"

Exp_t *exp_init(ExpType t, void *exp) {
    Exp_t *e = mem_calloc(1, sizeof(Exp_t));
    e->type = t;
    e->exp = exp;
    return e;
}

Exp_binary_t *exp_binary_init(Exp_t* left, Operator op, Exp_t * right) {
    Exp_binary_t *e = mem_calloc(1, sizeof(Exp_binary_t));
    e->op = op;
    e->right = right;
    e->left = left;
    return e;
}

Exp_unary_t *exp_unary_init(Operator op, Exp_t *rigth) {
    Exp_unary_t *e = mem_calloc(1, sizeof(Exp_unary_t));
    e->op = op;
    e->right = rigth;
    return e;
}

Exp_literal_t *exp_literal_init(LiteralType type, void* value) {
    Exp_literal_t *e = mem_calloc(1, sizeof(Exp_literal_t));
    e->type = type;
    e->value = value;
    return e;
}

Exp_grouping_t *exp_grouping_init(Exp_t* exp) {
    Exp_grouping_t *e = mem_calloc(1, sizeof(Exp_grouping_t));
    e->exp = exp;
    return e;
}

void exp_binary_destroy(Exp_binary_t *exp) {
    exp_destroy(exp->left);
    exp_destroy(exp->right);
    return; 
}

void exp_unary_destroy(Exp_unary_t *exp) {
    exp_destroy(exp->right);
    return;
}

void exp_literal_destroy(Exp_literal_t *exp) {
    if(exp->type != T_NIL)
        free(exp->value);
    return;
}

void exp_groping_destroy(Exp_grouping_t * exp) {
    exp_destroy(exp->exp);
    return;
}


void exp_destroy(Exp_t *exp) {
    switch(exp->type) {
        case EXP_BINARY:
            exp_binary_destroy((Exp_binary_t*)exp->exp);
            break;
        case EXP_UNARY:
            exp_unary_destroy((Exp_unary_t*)exp->exp);
            break;
        case EXP_LITERAL:
            exp_literal_destroy((Exp_literal_t *)exp->exp);
            break;
        case EXP_GROUPING:
            exp_groping_destroy((Exp_grouping_t*)exp->exp);
            break;
        case EXP_PANIC_MODE: 
            // nothing to do
            break;
    }
    free(exp->exp);
    free(exp);
}

Operator token_to_operator(Token t) {
    switch (t.type) {
        case EQUAL_EQUAL:
            return OP_EQUAL;
        case BANG_EQUAL:
            return OP_NOT_EQUAL;
        case LESS:
            return OP_LESS;
        case LESS_EQUAL:
            return OP_LESS_EQUAL;
        case GREATER:
            return OP_GREATER;
        case GREATER_EQUAL:
            return OP_GREATER_EQUAL;
        case PLUS:
            return OP_PLUS;
        case MINUS:
            return OP_MINUS;
        case STAR:
            return OP_STAR;
        case SLASH:
            return OP_SLASH;
        case AND:
            return OP_AND;
        case OR:
            return OP_OR;
        case BANG:
            return OP_NOT;
        default:
            return OP_ERROR;
    }
}

