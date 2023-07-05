#include "syntax.h"
#include "memory.h"
#include "token.h"
#include <string.h>

#pragma region Expressions

void *exp_unwrap(Exp_t *exp) {
    return exp->exp;
}

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
    switch(type) {
        case T_STRING: {
            e->value = strdup(value);
            break;
        }
        case T_NUMBER: {
            double *n = mem_calloc(1, sizeof(double));
            *n = atof(value);
            e->value = n;
            break;
        }    
        case T_NIL:
        case T_BOOLEAN: {
            e->value = value;
            break;
        }
    }
    return e;
}

Exp_grouping_t *exp_grouping_init(Exp_t* exp) {
    Exp_grouping_t *e = mem_calloc(1, sizeof(Exp_grouping_t));
    e->exp = exp;
    return e;
}

Exp_identifier_t *exp_identifier_init(char *identifier) {
    Exp_identifier_t *e = mem_calloc(1, sizeof(Exp_identifier_t));
    char *ide = mem_calloc(1, strlen(identifier) * sizeof(char));
    memcpy(ide, identifier, strlen(identifier) * sizeof(char));
    e->identifier = ide;
    return e;
}

void exp_binary_destroy(Exp_binary_t *exp) {
    exp_destroy(exp->left);
    exp_destroy(exp->right);
    free(exp);
    return; 
}

void exp_unary_destroy(Exp_unary_t *exp) {
    exp_destroy(exp->right);
    free(exp);
    return;
}

void exp_literal_destroy(Exp_literal_t *exp) {
    if(exp->type != T_NIL)
        free(exp->value);
    free(exp);
    return;
}

void exp_groping_destroy(Exp_grouping_t * exp) {
    exp_destroy(exp->exp);
    free(exp);
    return;
}

void exp_identifer_destroy(Exp_identifier_t *exp) {
    free(exp->identifier);
    free(exp);
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
        case EXP_IDENTIFIER:
            exp_identifer_destroy((Exp_identifier_t*)exp->exp);
            break;
        case EXP_PANIC_MODE: 
            free(exp->exp);
            break;
    }
    free(exp);
}

#pragma endregion Expressions

#pragma region Statements

Stmt_t *stmt_init(StmtType type, void *stmt, int line) {
    Stmt_t *s = mem_calloc(1, sizeof(Stmt_t));
    s->type = type;
    s->stmt = stmt;
    s->line = line;
    return s;
}

Stmt_print_t *stmt_print_init(Exp_t *exp) {
    Stmt_print_t *s = mem_calloc(1, sizeof(Stmt_print_t));
    s->exp = exp;
    return s;
}

Stmt_expr_t *stmt_expr_init(Exp_t *exp) {
    Stmt_expr_t *s = mem_calloc(1, sizeof(Stmt_expr_t));
    s->exp = exp;
    return s;
}

Stmt_conditional_t *stmt_conditional_init(Exp_t *exp_cond, Stmt_t *stmt_then, Stmt_t *stmt_else) {
    Stmt_conditional_t *s = mem_calloc(1, sizeof(Stmt_conditional_t));
    s->condition = exp_cond;
    s->then_brench = stmt_then;
    s->else_brench = stmt_else;
    return s;
}

Stmt_block_t *stmt_block_init(List stmts) {
    Stmt_block_t *s = mem_calloc(1, sizeof(Stmt_block_t));
    s->statements = stmts;
    return s;
}

Stmt_declaration_t *stmt_declaration_init(char *identifier, Exp_t *exp) {
    Stmt_declaration_t *s = mem_calloc(1, sizeof(Stmt_declaration_t));
    char *ide = mem_calloc(1, strlen(identifier)*sizeof(char));
    memcpy(ide, identifier, strlen(identifier)*sizeof(char));
    s->identifier = ide;
    s->exp = exp;
    return s;
}

Stmt_assignement_t *stmt_assignement_init(char *identifier, Exp_t *exp) {
    return stmt_declaration_init(identifier, exp);
}

void stmt_print_destroy(Stmt_print_t * stmt) {
    exp_destroy(stmt->exp);
    free(stmt);
    return;
}

void stmt_expr_destroy(Stmt_expr_t * stmt) {
    exp_destroy(stmt->exp);
    free(stmt);
    return;
}

void stmt_conditional_destroy(Stmt_conditional_t *stmt) {
    exp_destroy(stmt->condition);
    stmt_destroy(stmt->then_brench);
    if(stmt->else_brench != NULL)
        stmt_destroy(stmt->else_brench);
    free(stmt);
    return;
}

void stmt_block_destroy(Stmt_block_t *stmt) {
    list_free(stmt->statements, stmt_free);
    free(stmt);
    return;
}

void stmt_declaration_destroy(Stmt_declaration_t *stmt) {
    exp_destroy(stmt->exp);
    free(stmt->identifier);
    free(stmt);
    return;
}

void stmt_assignement_destroy(Stmt_assignement_t *stmt) {
    stmt_declaration_destroy(stmt);
    return;
}

void stmt_free(void* s) {
    stmt_destroy(s);
}


void *stmt_unwrap(Stmt_t *s) {
    return s->stmt;
}

void stmt_destroy(Stmt_t *stmt) {
    switch(stmt->type) {
        case STMT_PRINT:
            stmt_print_destroy((Stmt_print_t*)stmt->stmt);
            break;
        case STMT_EXPR:
            stmt_expr_destroy((Stmt_expr_t*)stmt->stmt);
            break;
        case STMT_IF:
            stmt_conditional_destroy((Stmt_conditional_t*)stmt->stmt);
            break;
        case STMT_DECLARATION:
            stmt_declaration_destroy((Stmt_declaration_t*)stmt->stmt);
            break;
        case STMT_ASSIGNMENT:
            stmt_assignement_destroy((Stmt_assignement_t*)stmt->stmt);
    }

    free(stmt);
    return;
}

#pragma  endregion Statements

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
        case MOD:
            return OP_MOD;
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

