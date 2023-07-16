#include "syntax.h"
#include "list.h"
#include "memory.h"
#include "token.h"
#include <string.h>

#pragma region Expressions

void *exp_unwrap(exp_t *exp) {
    return exp->exp;
}

exp_t *exp_init(exp_type_t t, void *exp) {
    exp_t *e = mem_calloc(1, sizeof(exp_t));
    e->type = t;
    e->exp = exp;
    return e;
}

exp_t *exp_dup(exp_t *exp) {
    exp_t *duped = mem_calloc(1, sizeof(exp_t));
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

exp_binary_t *exp_binary_init(exp_t* left, operator_t op, exp_t * right) {
    exp_binary_t *e = mem_calloc(1, sizeof(exp_binary_t));
    e->op = op;
    e->right = right;
    e->left = left;
    return e;
}

exp_binary_t *exp_binary_dup(exp_binary_t *exp) {
    exp_binary_t *duped = mem_calloc(1, sizeof(exp_binary_t));
    duped->op = exp->op;
    duped->left = exp_dup(exp->left);
    duped->right = exp_dup(exp->right);
    return duped;
}

exp_unary_t *exp_unary_init(operator_t op, exp_t *right) {
    exp_unary_t *e = mem_calloc(1, sizeof(exp_unary_t));
    e->op = op;
    e->right = right;
    return e;
}

exp_unary_t *exp_unary_dup(exp_unary_t* exp) {
    exp_unary_t *duped = mem_calloc(1, sizeof(exp_unary_t));
    duped->op = exp->op;
    duped->right = exp_dup(exp->right);
    return duped;
}

exp_literal_t *exp_literal_init(literal_type_t type, void* value) {
    exp_literal_t *e = mem_calloc(1, sizeof(exp_literal_t));
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

exp_literal_t *exp_literal_dup(exp_literal_t *exp) {
    exp_literal_t *duped = mem_calloc(1, sizeof(exp_literal_t));
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

exp_grouping_t *exp_grouping_init(exp_t* exp) {
    exp_grouping_t *e = mem_calloc(1, sizeof(exp_grouping_t));
    e->exp = exp;
    return e;
}

exp_grouping_t *exp_grouping_dup(exp_grouping_t *exp) {
    exp_grouping_t *duped = mem_calloc(1, sizeof(exp_grouping_t));
    duped->exp = exp_dup(exp->exp);
    return duped;
}

exp_identifier_t *exp_identifier_init(char *identifier) {
    exp_identifier_t *e = mem_calloc(1, sizeof(exp_identifier_t));
    char *ide = mem_calloc(1, strlen(identifier) * sizeof(char));
    memcpy(ide, identifier, strlen(identifier) * sizeof(char));
    e->identifier = ide;
    return e;
}

exp_identifier_t *exp_identifier_dup(exp_identifier_t *exp) {
    exp_identifier_t *duped = mem_calloc(1, sizeof(exp_identifier_t));
    duped->identifier = strdup(exp->identifier);
    return duped;
}

exp_call_t *exp_call_init(char *identifier, l_list_t actuals) {
    exp_call_t *e = mem_calloc(1, sizeof(exp_call_t));
    char *ide = mem_calloc(1, strlen(identifier) * sizeof(char));
    memcpy(ide, identifier, strlen(identifier) * sizeof(char));
    e->identifier = ide;
    e->actuals = actuals;
    return e;
}

exp_call_t *exp_call_dup(exp_call_t *exp) {
    exp_call_t *duped = mem_calloc(1, sizeof(exp_call_t));
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

void exp_binary_destroy(exp_binary_t *exp) {
    exp_destroy(exp->left);
    exp_destroy(exp->right);
    mem_free(exp);
    return; 
}

void exp_unary_destroy(exp_unary_t *exp) {
    exp_destroy(exp->right);
    mem_free(exp);
    return;
}

void exp_literal_destroy(exp_literal_t *exp) {
    mem_free(exp->value);
    mem_free(exp);
    return;
}

void exp_groping_destroy(exp_grouping_t * exp) {
    exp_destroy(exp->exp);
    mem_free(exp);
    return;
}

void exp_identifier_destroy(exp_identifier_t *exp) {
    mem_free(exp->identifier);
    mem_free(exp);
    return;
}

void exp_call_destroy(exp_call_t *exp) {
    mem_free(exp->identifier);
    list_free(exp->actuals, exp_free);
    mem_free(exp);
}

void exp_destroy(exp_t *exp) {
    switch(exp->type) {
        case EXP_BINARY:
            exp_binary_destroy((exp_binary_t*)exp->exp);
            break;
        case EXP_UNARY:
            exp_unary_destroy((exp_unary_t*)exp->exp);
            break;
        case EXP_LITERAL:
            exp_literal_destroy((exp_literal_t *)exp->exp);
            break;
        case EXP_GROUPING:
            exp_groping_destroy((exp_grouping_t*)exp->exp);
            break;
        case EXP_IDENTIFIER:
            exp_identifier_destroy((exp_identifier_t*)exp->exp);
            break;
        case EXP_CALL:
            exp_call_destroy((exp_call_t*)exp->exp);
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

stmt_t *stmt_init(stmt_type_t type, void *stmt, int line) {
    stmt_t *s = mem_calloc(1, sizeof(stmt_t));
    s->type = type;
    s->stmt = stmt;
    s->line = line;
    return s;
}

stmt_t *stmt_dup(stmt_t *stmt) {
    stmt_t *duped = mem_calloc(1, sizeof(stmt_t));
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

stmt_print_t *stmt_print_init(exp_t *exp) {
    stmt_print_t *s = mem_calloc(1, sizeof(stmt_print_t));
    s->exp = exp;
    return s;
}

stmt_print_t *stmt_print_dup(stmt_print_t *s) {
    stmt_print_t *duped = mem_calloc(1, sizeof(stmt_print_t));
    duped->exp = exp_dup(s->exp);
    return duped;
}

stmt_expr_t *stmt_expr_init(exp_t *exp) {
    stmt_expr_t *s = mem_calloc(1, sizeof(stmt_expr_t));
    s->exp = exp;
    return s;
}

stmt_expr_t *stmt_expr_dup(stmt_expr_t *s) {
    stmt_expr_t *duped = mem_calloc(1, sizeof(stmt_expr_t));
    duped->exp = exp_dup(s->exp);
    return duped;
}

stmt_conditional_t *stmt_conditional_init(exp_t *exp_cond, stmt_t *stmt_then, stmt_t *stmt_else) {
    stmt_conditional_t *s = mem_calloc(1, sizeof(stmt_conditional_t));
    s->condition = exp_cond;
    s->then_branch = stmt_then;
    s->else_branch = stmt_else;
    return s;
}

stmt_conditional_t *stmt_conditional_dup(stmt_conditional_t *s) {
    stmt_conditional_t *duped = mem_calloc(1, sizeof(stmt_conditional_t));
    duped->condition = exp_dup(s->condition);
    duped->then_branch = stmt_dup(s->then_branch);
    if(s->else_branch)
        duped->else_branch = stmt_dup(s->else_branch);
    else
        duped->else_branch = NULL;
    return duped;
}

stmt_block_t *stmt_block_init(l_list_t stmts) {
    stmt_block_t *s = mem_calloc(1, sizeof(stmt_block_t));
    s->statements = stmts;
    return s;
}

stmt_block_t *stmt_block_dup(stmt_block_t *s) {
    stmt_block_t *duped = mem_calloc(1, sizeof(stmt_block_t));
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

stmt_declaration_t *stmt_declaration_init(char *identifier, exp_t *exp) {
    stmt_declaration_t *s = mem_calloc(1, sizeof(stmt_declaration_t));
    char *ide = mem_calloc(1, strlen(identifier)*sizeof(char));
    memcpy(ide, identifier, strlen(identifier)*sizeof(char));
    s->identifier = ide;
    s->exp = exp;
    return s;
}

stmt_declaration_t *stmt_declaration_dup(stmt_declaration_t *s) {
    stmt_declaration_t *duped = mem_calloc(1, sizeof(stmt_declaration_t));
    duped->identifier = strdup(s->identifier);
    duped->exp = exp_dup(s->exp);
    return duped;
}

stmt_assignment_t *stmt_assignment_init(char *identifier, exp_t *exp) {
    return stmt_declaration_init(identifier, exp);
}

stmt_assignment_t *stmt_assignment_dup(stmt_assignment_t *s) {
    return stmt_declaration_dup(s);
}

stmt_function_t *stmt_function_init(char *identifier, l_list_t formals, stmt_t *body) {
    stmt_function_t *s = mem_calloc(1, sizeof(stmt_function_t));
    char *ide = mem_calloc(1, strlen(identifier) * sizeof(char));
    memcpy(ide, identifier, strlen(identifier) * sizeof(char));
    s->identifier = ide;
    s->formals = formals;
    s->body = body;
    return s;
}

stmt_function_t *stmt_function_dup(stmt_function_t *s) {
    stmt_function_t *duped = mem_calloc(1, sizeof(stmt_function_t));
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

void stmt_print_destroy(stmt_print_t * stmt) {
    exp_destroy(stmt->exp);
    mem_free(stmt);
    return;
}

void stmt_expr_destroy(stmt_expr_t * stmt) {
    exp_destroy(stmt->exp);
    mem_free(stmt);
    return;
}

void stmt_conditional_destroy(stmt_conditional_t *stmt) {
    exp_destroy(stmt->condition);
    stmt_destroy(stmt->then_branch);
    if(stmt->else_branch != NULL)
        stmt_destroy(stmt->else_branch);
    mem_free(stmt);
    return;
}

void stmt_block_destroy(stmt_block_t *stmt) {
    list_free(stmt->statements, stmt_free);
    mem_free(stmt);
    return;
}

void stmt_declaration_destroy(stmt_declaration_t *stmt) {
    exp_destroy(stmt->exp);
    mem_free(stmt->identifier);
    mem_free(stmt);
    return;
}

void stmt_assignment_destroy(stmt_assignment_t *stmt) {
    stmt_declaration_destroy(stmt);
    return;
}

void stmt_function_destroy(stmt_function_t *stmt) {
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


void *stmt_unwrap(stmt_t *s) {
    return s->stmt;
}

void stmt_destroy(stmt_t *stmt) {
    switch(stmt->type) {
        case STMT_PRINT:
            stmt_print_destroy((stmt_print_t*)stmt->stmt);
            break;
        case STMT_EXPR:
            stmt_expr_destroy((stmt_expr_t*)stmt->stmt);
            break;
        case STMT_IF:
            stmt_conditional_destroy((stmt_conditional_t*)stmt->stmt);
            break;
        case STMT_DECLARATION:
            stmt_declaration_destroy((stmt_declaration_t*)stmt->stmt);
            break;
        case STMT_ASSIGNMENT:
            stmt_assignment_destroy((stmt_assignment_t*)stmt->stmt);
            break;
        case STMT_FUN:
            stmt_function_destroy((stmt_function_t*)stmt->stmt);
            break;
        case STMT_BLOCK:
            stmt_block_destroy((stmt_block_t * )stmt->stmt);
            break;
    }

    mem_free(stmt);
    return;
}

#pragma endregion Statements

operator_t token_to_operator(token_t t) {
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
        case PIPE_GREATER:
            return OP_FORWARD;
        default:
            return OP_ERROR;
    }
}

char * literal_type_to_string(literal_type_t type) {
    switch(type) {
        case T_STRING:
            return "String";
        case T_BOOLEAN:
            return "Boolean";
        case T_NUMBER:
            return "Number";
        case T_NIL:
            return "Nil";
        case T_CLOSURE:
            return "Function";
    }    
}