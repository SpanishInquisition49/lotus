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
static Value *eval_identifier(Interpreter*, Exp_t*);
static Value *eval_call(Interpreter*, Exp_t*);

static Value *eval_stmt(Interpreter*, Stmt_t*);
static Value *eval_stmt_exp(Interpreter*, Stmt_t*);
static Value *eval_stmt_print(Interpreter*, Stmt_t*);
static Value *eval_stmt_conditional(Interpreter*, Stmt_t*);
static Value *eval_stmt_block(Interpreter*, Stmt_t*);
static Value *eval_stmt_declaration(Interpreter*, Stmt_t*);
static Value *eval_stmt_assignment(Interpreter*, Stmt_t*);
static Value *eval_stmt_function(Interpreter*, Stmt_t*);

static int is_equal(Interpreter*, Value*, Value*);
static int is_truthy(Interpreter*, Value*);
static void bulk_pretty_print(List);
static void pretty_print(Value*);
static Value *return_null(void);
static Value *value_clone(Value*);
static void value_destroy(Value*);

static void raise_runtime_error(Interpreter*, char*);

void interpreter_init(Interpreter *interpreter, List statements) {
    interpreter->statements = statements;
    interpreter->environment = mem_calloc(1, sizeof(Env));
    interpreter->environment->identifier = NULL;
    interpreter->environment->value = NULL;
    interpreter->environment->prev = NULL;
    return;
}

void interpreter_destroy(Interpreter interpreter) {
    list_free(interpreter.statements, stmt_free);
    env_destroy(interpreter.environment);
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

Value *eval_stmt(Interpreter *i, Stmt_t* s) {
    switch (s->type) {
        case STMT_EXPR:
            return eval_stmt_exp(i, s);
        case STMT_PRINT:
            return eval_stmt_print(i, s);
        case STMT_IF:
            return eval_stmt_conditional(i, s);
        case STMT_BLOCK:
            return eval_stmt_block(i, s);
        case STMT_DECLARATION:
            return eval_stmt_declaration(i, s);
        case STMT_ASSIGNMENT:
            return eval_stmt_assignment(i, s);
        case STMT_FUN:
            return eval_stmt_function(i, s);
        default:
            raise_runtime_error(i, "Unimplemented Error\n");
    }
    return return_null();
}

Value *eval(Interpreter* i, Exp_t *exp) {
    switch(exp->type) {
        case EXP_UNARY:
            return value_clone(eval_unary(i, exp));
        case EXP_BINARY:
            return value_clone(eval_binary(i, exp));
        case EXP_GROUPING:
            return value_clone(eval_grouping(i, exp));
        case EXP_LITERAL:
            return value_clone(eval_literal(exp));
        case EXP_IDENTIFIER:
            return value_clone(eval_identifier(i, exp));
        case EXP_CALL:
            return value_clone(eval_call(i, exp));
        default: // Unreachable
            raise_runtime_error(i, "Unknown expression\n");
    }

    // Unreachable
    return NULL;
}

Value *eval_literal(Exp_t *exp) {
    return (Value*)exp->exp;
}

Value *eval_identifier(Interpreter *i, Exp_t *exp){
    Exp_identifier_t *unwrapped_exp = exp_unwrap(exp);
    Value *v = env_get(i->environment, unwrapped_exp->identifier);
    if(v == NULL)
        raise_runtime_error(i, "Undeclared identifier\n");
    return v;
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
            //exp_destroy(unwrapped_exp->right);
            right = exp_literal_init(T_BOOLEAN, value);
            //unwrapped_exp->right = exp_init(EXP_LITERAL, right);
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
            //exp_destroy(unwrapped_exp->left);
            //unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_GREATER_EQUAL: {
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) >= *((double*)right->value);
            //exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            //unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_LESS: {
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) < *((double*)right->value);
            //exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            //unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_LESS_EQUAL: {
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            int *value = mem_calloc(1, sizeof(int));
            *value = *((double*)left->value) <= *((double*)right->value);
            //exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            //unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_EQUAL: {
            int *value = mem_calloc(1, sizeof(int));
            *value = is_equal(i, left, right);
            //exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            //unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        case OP_NOT_EQUAL: {
            int *value = mem_calloc(1, sizeof(int));
            *value = !is_equal(i, left, right);
            //exp_destroy(unwrapped_exp->left);
            left = exp_literal_init(T_BOOLEAN, value);
            //unwrapped_exp->left = exp_init(EXP_LITERAL, left);
            return left;
        }
        default: // Theoretically unreachable
            raise_runtime_error(i, "Unkown Operation\n");
    }

    // Unreachable
    return NULL;
}

Value *eval_call(Interpreter *i, Exp_t *exp) {
    Exp_call_t *unwrapped_exp = exp_unwrap(exp);
    List values = NULL;
    List expressions = unwrapped_exp->actuals;
    while(expressions != NULL) {
        list_add(&values, eval(i, expressions->data));
        expressions = expressions->next;
    }
    list_reverse_in_place(&values);
    Value *v = env_get(i->environment, unwrapped_exp->identifier);
    if(v->type != T_CLOSURE)
        raise_runtime_error(i, "The identifier is not a function name\n");
    closure_t *closure = (closure_t*)v->value;
    Env *old_env = i->environment;
    i->environment = env_bulk_bind(i->environment, closure->formals, values);
    Value *res = eval_stmt(i, closure->body);
    i->environment = env_restore(i->environment, old_env);
    return res;
}

Value *eval_stmt_exp(Interpreter *i, Stmt_t *s) {
    Stmt_expr_t *unwrapped_stmt = stmt_unwrap(s);
    return eval(i, unwrapped_stmt->exp);
}

Value *eval_stmt_print(Interpreter *i, Stmt_t *s) {
    Stmt_print_t *unwrapped_stmt = stmt_unwrap(s);
    Value *v = eval(i, unwrapped_stmt->exp);
    pretty_print(v);
    return return_null(); 
}

Value *eval_stmt_conditional(Interpreter *i, Stmt_t *s) {
    Stmt_conditional_t *unwrapped_stmt = stmt_unwrap(s);
    Value *cond = eval(i, unwrapped_stmt->condition);
    Value *res = return_null();
    if(is_truthy(i, cond))
        res = eval_stmt(i, unwrapped_stmt->then_brench);
    else if(unwrapped_stmt->else_brench != NULL)
        res = eval_stmt(i, unwrapped_stmt->else_brench);
    return res;
}

Value *eval_stmt_block(Interpreter *i, Stmt_t *s) {
    Stmt_block_t *unwrapped_stmt = stmt_unwrap(s);
    List stmts = unwrapped_stmt->statements;
    Env *pre = i->environment;
    Value *v = return_null();
    while(stmts) {
        v = eval_stmt(i, stmts->data);
        stmts = stmts->next;
    }
    i->environment = env_restore(i->environment, pre);
    return v;
}

Value *eval_stmt_declaration(Interpreter *i, Stmt_t *s) {
    Stmt_declaration_t *unwrapped_stmt = stmt_unwrap(s);
    Value *v = eval(i, unwrapped_stmt->exp);
    i->environment = env_bind(i->environment, unwrapped_stmt->identifier, v);
    return return_null();
}

Value *eval_stmt_assignment(Interpreter *i, Stmt_t *s) {
    Stmt_assignment_t *unwrapped_stmt = stmt_unwrap(s);
    Value *res = eval(i, unwrapped_stmt->exp);
    Value *old = env_set(i->environment, unwrapped_stmt->identifier, res);
    value_destroy(old);
    return return_null();
}

Value *eval_stmt_function(Interpreter *i, Stmt_t *s) {
    Stmt_function_t *unwrapped_stmt = stmt_unwrap(s);
    closure_t *closure = mem_calloc(1, sizeof(closure_t));
    closure->identifier = strdup(unwrapped_stmt->identifier);
    closure->formals = list_dup(unwrapped_stmt->formals);
    closure->body = unwrapped_stmt->body;
    Value *v = mem_calloc(1, sizeof(Value));
    v->type = T_CLOSURE;
    v->value = closure;
    unwrapped_stmt->body = NULL;
    i->environment = env_bind(i->environment, unwrapped_stmt->identifier, v);
    return return_null();
}

Value *value_clone(Value *v) {
    Value *val = mem_calloc(1, sizeof(Value));
    val->type = v->type;
    switch(v->type) {
        case T_NUMBER: {
            double *n = mem_calloc(1, sizeof(double));
            *n = *((double*)v->value);
            val->value = n;
            break;
        }
        case T_BOOLEAN: {
            int *n = mem_calloc(1, sizeof(int));
            *n = *((int*)v->value);
            val->value = n;
            break;
        }
        case T_STRING: {
            char * s = mem_calloc(1,strlen(v->value) * sizeof(char));
            memcpy(s, v->value, strlen(v->value) * sizeof(char));
            val->value = s;
            break;
        }
        case T_NIL:
            val->value = NULL;
            break;
    }
    return val;
}

Value *return_null(void) {
    Value *v = mem_calloc(1, sizeof(Value));
    v->type = T_NIL;
    v->value = NULL;
    return v;
}

void value_destroy(Value *v) {
    if(v->type != T_NIL)
        free(v->value);
    free(v);
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
            if( fmod(*((double*)v->value), 1) == 0)
                printf("%.0f\n", *((double*)v->value));
            else
                printf("%.2f\n", *((double*)v->value));
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


void bulk_pretty_print(List values) {
    List value = values;
    while(value != NULL) {
        pretty_print(value->data);
        value = value->next;
    }
    return;
}

void raise_runtime_error(Interpreter *i, char *msg) {
    Log(ERROR, msg);
    interpreter_destroy(*i);
    exit(EXIT_FAILURE);
}

