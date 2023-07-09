#include "syntax.h"
#include "list.h"
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

Exp_t *exp_dup(Exp_t *exp) {
    Exp_t *duped = mem_calloc(1, sizeof(Exp_t));
    duped->type = exp->type;
    switch(exp->type) {
        case EXP_UNARY:
            duped->exp = exp_unary_dup(exp->exp);
            break;
        case EXP_BINARY:
            duped->exp = exp_binary_dup(exp->exp);
            break;
        case EXP_LITERAL:
            duped->exp = exp_literal_dup(exp->exp);
            break;
        case EXP_IDENTIFIER:
            duped->exp = exp_identifier_dup(exp->exp);
            break;
        case EXP_GROUPING:
            duped->exp = exp_grouping_dup(exp->exp);
            break;
        case EXP_CALL:
            duped->exp = exp_call_dup(exp->exp);
            break;
    }

    return duped;
}

Exp_binary_t *exp_binary_init(Exp_t* left, Operator op, Exp_t * right) {
    Exp_binary_t *e = mem_calloc(1, sizeof(Exp_binary_t));
    e->op = op;
    e->right = right;
    e->left = left;
    return e;
}

Exp_binary_t *exp_binary_dup(Exp_binary_t *exp) {
    Exp_binary_t *duped = mem_calloc(1, sizeof(Exp_binary_t));
    duped->op = exp->op;
    duped->left = exp_dup(exp->left);
    duped->right = exp_dup(exp->right);
    return duped;
}

Exp_unary_t *exp_unary_init(Operator op, Exp_t *rigth) {
    Exp_unary_t *e = mem_calloc(1, sizeof(Exp_unary_t));
    e->op = op;
    e->right = rigth;
    return e;
}

Exp_unary_t *exp_unary_dup(Exp_unary_t* exp) {
    Exp_unary_t *duped = mem_calloc(1, sizeof(Exp_unary_t));
    duped->op = exp->op;
    duped->right = exp_dup(exp->right);
    return duped;
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

Exp_literal_t *exp_literal_dup(Exp_literal_t *exp) {
    Exp_literal_t *duped = mem_calloc(1, sizeof(Exp_literal_t));
    duped->type = exp->type;
    switch(exp->type) {
        case T_STRING:
            duped->value = strdup(exp->value);
            break;
        case T_NUMBER: {
            double *n = mem_calloc(1, sizeof(double));
            *n = *((double*)exp->value);
            duped->value = n;
            break;
        }
        case T_BOOLEAN: {
            int *n = mem_calloc(1, sizeof(int));
            *n = *((int*)exp->value);
            duped->value = n;
            break;
        }
        case T_NIL:
            duped->value = NULL;
            break;
    }
    return duped;
}

Exp_grouping_t *exp_grouping_init(Exp_t* exp) {
    Exp_grouping_t *e = mem_calloc(1, sizeof(Exp_grouping_t));
    e->exp = exp;
    return e;
}

Exp_grouping_t *exp_grouping_dup(Exp_grouping_t *exp) {
    Exp_grouping_t *duped = mem_calloc(1, sizeof(Exp_grouping_t));
    duped->exp = exp_dup(exp->exp);
    return duped;
}

Exp_identifier_t *exp_identifier_init(char *identifier) {
    Exp_identifier_t *e = mem_calloc(1, sizeof(Exp_identifier_t));
    char *ide = mem_calloc(1, strlen(identifier) * sizeof(char));
    memcpy(ide, identifier, strlen(identifier) * sizeof(char));
    e->identifier = ide;
    return e;
}

Exp_identifier_t *exp_identifier_dup(Exp_identifier_t *exp) {
    Exp_identifier_t *duped = mem_calloc(1, sizeof(Exp_identifier_t));
    duped->identifier = strdup(exp->identifier);
    return duped;
}

Exp_call_t *exp_call_init(char *identifier, l_list_t actuals) {
    Exp_call_t *e = mem_calloc(1, sizeof(Exp_call_t));
    char *ide = mem_calloc(1, strlen(identifier) * sizeof(char));
    memcpy(ide, identifier, strlen(identifier) * sizeof(char));
    e->identifier = ide;
    e->actuals = actuals;
    return e;
}

Exp_call_t *exp_call_dup(Exp_call_t *exp) {
    Exp_call_t *duped = mem_calloc(1, sizeof(Exp_call_t));
    duped->identifier = strdup(exp->identifier);
    l_list_t act = NULL;
    l_list_t current = exp->actuals;
    while(current) {
        list_add(&act, exp_dup(current->data));
        current = current->next;
    }
    list_reverse_in_place(&act);
    duped->actuals = act;
    return duped;
}

void exp_binary_destroy(Exp_binary_t *exp) {
    exp_destroy(exp->left);
    exp_destroy(exp->right);
    mem_free(exp);
    return; 
}

void exp_unary_destroy(Exp_unary_t *exp) {
    exp_destroy(exp->right);
    mem_free(exp);
    return;
}

void exp_literal_destroy(Exp_literal_t *exp) {
    mem_free(exp->value);
    mem_free(exp);
    return;
}

void exp_groping_destroy(Exp_grouping_t * exp) {
    exp_destroy(exp->exp);
    mem_free(exp);
    return;
}

void exp_identifier_destroy(Exp_identifier_t *exp) {
    mem_free(exp->identifier);
    mem_free(exp);
    return;
}

void exp_call_destroy(Exp_call_t *exp) {
    mem_free(exp->identifier);
    list_free(exp->actuals, exp_free);
    mem_free(exp);
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
            exp_identifier_destroy((Exp_identifier_t*)exp->exp);
            break;
        case EXP_CALL:
            exp_call_destroy((Exp_call_t*)exp->exp);
            break;
        case EXP_PANIC_MODE: 
            mem_free(exp->exp);
            break;
    }
    mem_free(exp);
}

void exp_free(void* e) {
    exp_destroy(e);
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

Stmt_t *stmt_dup(Stmt_t *stmt) {
    Stmt_t *duped = mem_calloc(1, sizeof(Stmt_t));
    duped->type = stmt->type;
    duped->line = stmt->line;
    switch(stmt->type) {
        case STMT_IF:
            duped->stmt = stmt_conditional_dup(stmt->stmt);
            break;
        case STMT_FUN:
            duped->stmt = stmt_function_dup(stmt->stmt);
            break;
        case STMT_DECLARATION:
            duped->stmt = stmt_declaration_dup(stmt->stmt);
            break;
        case STMT_EXPR:
            duped->stmt = stmt_expr_dup(stmt->stmt);
            break;
        case STMT_PRINT:
            duped->stmt = stmt_print_dup(stmt->stmt);
            break;
        case STMT_BLOCK:
            duped->stmt = stmt_block_dup(stmt->stmt);
            break;
        case STMT_ASSIGNMENT:
            duped->stmt = stmt_assignment_dup(stmt->stmt);
            break;
    }
    return duped;
}

Stmt_print_t *stmt_print_init(Exp_t *exp) {
    Stmt_print_t *s = mem_calloc(1, sizeof(Stmt_print_t));
    s->exp = exp;
    return s;
}

Stmt_print_t *stmt_print_dup(Stmt_print_t *s) {
    Stmt_print_t *duped = mem_calloc(1, sizeof(Stmt_print_t));
    duped->exp = exp_dup(s->exp);
    return duped;
}

Stmt_expr_t *stmt_expr_init(Exp_t *exp) {
    Stmt_expr_t *s = mem_calloc(1, sizeof(Stmt_expr_t));
    s->exp = exp;
    return s;
}

Stmt_expr_t *stmt_expr_dup(Stmt_expr_t *s) {
    Stmt_expr_t *duped = mem_calloc(1, sizeof(Stmt_expr_t));
    duped->exp = exp_dup(s->exp);
    return duped;
}

Stmt_conditional_t *stmt_conditional_init(Exp_t *exp_cond, Stmt_t *stmt_then, Stmt_t *stmt_else) {
    Stmt_conditional_t *s = mem_calloc(1, sizeof(Stmt_conditional_t));
    s->condition = exp_cond;
    s->then_brench = stmt_then;
    s->else_brench = stmt_else;
    return s;
}

Stmt_conditional_t *stmt_conditional_dup(Stmt_conditional_t *s) {
    Stmt_conditional_t *duped = mem_calloc(1, sizeof(Stmt_conditional_t));
    duped->condition = exp_dup(s->condition);
    duped->then_brench = stmt_dup(s->then_brench);
    duped->else_brench = stmt_dup(s->else_brench);
    return duped;
}

Stmt_block_t *stmt_block_init(l_list_t stmts) {
    Stmt_block_t *s = mem_calloc(1, sizeof(Stmt_block_t));
    s->statements = stmts;
    return s;
}

Stmt_block_t *stmt_block_dup(Stmt_block_t *s) {
    Stmt_block_t *duped = mem_calloc(1, sizeof(Stmt_block_t));
    l_list_t stmts = NULL;
    l_list_t current = s->statements;
    while(current) {
        list_add(&stmts, stmt_dup(current->data));
        current = current->next;
    }
    list_reverse_in_place(&stmts);
    duped->statements = stmts;
    return duped;
}

Stmt_declaration_t *stmt_declaration_init(char *identifier, Exp_t *exp) {
    Stmt_declaration_t *s = mem_calloc(1, sizeof(Stmt_declaration_t));
    char *ide = mem_calloc(1, strlen(identifier)*sizeof(char));
    memcpy(ide, identifier, strlen(identifier)*sizeof(char));
    s->identifier = ide;
    s->exp = exp;
    return s;
}

Stmt_declaration_t *stmt_declaration_dup(Stmt_declaration_t *s) {
    Stmt_declaration_t *duped = mem_calloc(1, sizeof(Stmt_declaration_t));
    duped->identifier = strdup(s->identifier);
    duped->exp = exp_dup(s->exp);
    return duped;
}

Stmt_assignment_t *stmt_assignment_init(char *identifier, Exp_t *exp) {
    return stmt_declaration_init(identifier, exp);
}

Stmt_assignment_t *stmt_assignment_dup(Stmt_assignment_t *s) {
    return stmt_declaration_dup(s);
}

Stmt_function_t *stmt_function_init(char *identifier, l_list_t formals, Stmt_t *body) {
    Stmt_function_t *s = mem_calloc(1, sizeof(Stmt_function_t));
    char *ide = mem_calloc(1, strlen(identifier) * sizeof(char));
    memcpy(ide, identifier, strlen(identifier) * sizeof(char));
    s->identifier = ide;
    s->formals = formals;
    s->body = body;
    return s;
}

Stmt_function_t *stmt_function_dup(Stmt_function_t *s) {
    Stmt_function_t *duped = mem_calloc(1, sizeof(Stmt_function_t));
    duped->identifier = strdup(s->identifier);
    duped->body = stmt_dup(s->body);
    l_list_t f = NULL;
    l_list_t current = s->formals;
    while(current) {
        list_add(&f, strdup(current->data));
        current = current->next;
    }
    list_reverse_in_place(&f);
    duped->formals = f;
    return duped;
}

void stmt_print_destroy(Stmt_print_t * stmt) {
    exp_destroy(stmt->exp);
    mem_free(stmt);
    return;
}

void stmt_expr_destroy(Stmt_expr_t * stmt) {
    exp_destroy(stmt->exp);
    mem_free(stmt);
    return;
}

void stmt_conditional_destroy(Stmt_conditional_t *stmt) {
    exp_destroy(stmt->condition);
    stmt_destroy(stmt->then_brench);
    if(stmt->else_brench != NULL)
        stmt_destroy(stmt->else_brench);
    mem_free(stmt);
    return;
}

void stmt_block_destroy(Stmt_block_t *stmt) {
    list_free(stmt->statements, stmt_free);
    mem_free(stmt);
    return;
}

void stmt_declaration_destroy(Stmt_declaration_t *stmt) {
    exp_destroy(stmt->exp);
    mem_free(stmt->identifier);
    mem_free(stmt);
    return;
}

void stmt_assignment_destroy(Stmt_assignment_t *stmt) {
    stmt_declaration_destroy(stmt);
    return;
}

void stmt_function_destroy(Stmt_function_t *stmt) {
    list_free(stmt->formals, NULL);
    if(stmt->body != NULL)
        stmt_destroy(stmt->body);
    mem_free(stmt->identifier);
    mem_free(stmt);
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
            stmt_assignment_destroy((Stmt_assignment_t*)stmt->stmt);
            break;
        case STMT_FUN:
            stmt_function_destroy((Stmt_function_t*)stmt->stmt);
            break;
        case STMT_BLOCK:
            stmt_block_destroy((Stmt_block_t * )stmt->stmt);
            break;
    }

    mem_free(stmt);
    return;
}

#pragma endregion Statements

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

