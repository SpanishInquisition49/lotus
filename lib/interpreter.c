#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "garbage.h"
#include "interpreter.h"
#include "list.h"
#include "syntax.h"
#include "memory.h"
#include "errors.h"

static value_t *eval(Interpreter*, Exp_t*);
static value_t *eval_literal(Interpreter *, Exp_t*);
static value_t *eval_grouping(Interpreter*, Exp_t*);
static value_t *eval_unary(Interpreter*, Exp_t*);
static value_t *eval_binary(Interpreter*, Exp_t*);
static value_t *eval_identifier(Interpreter*, Exp_t*);
static value_t *eval_call(Interpreter*, Exp_t*);

static value_t *eval_stmt(Interpreter*, Stmt_t*);
static value_t *eval_stmt_exp(Interpreter*, Stmt_t*);
static value_t *eval_stmt_print(Interpreter*, Stmt_t*);
static value_t *eval_stmt_conditional(Interpreter*, Stmt_t*);
static value_t *eval_stmt_block(Interpreter*, Stmt_t*);
static value_t *eval_stmt_declaration(Interpreter*, Stmt_t*);
static value_t *eval_stmt_assignment(Interpreter*, Stmt_t*);
static value_t *eval_stmt_function(Interpreter*, Stmt_t*);
static value_t *return_null(Interpreter*);

static int is_equal(Interpreter*, value_t*, value_t*);
static int is_truthy(Interpreter*, value_t*);
static void bulk_pretty_print(l_list_t);
static void pretty_print(value_t*);
static void value_destroy(value_t*);

static void raise_runtime_error(Interpreter*, char*, ...);

void interpreter_init(Interpreter *interpreter, Env *env,l_list_t statements, garbage_collector_t *garbage_collector) {
    interpreter->statements = statements;
    interpreter->environment = env;
    interpreter->garbage_collector = garbage_collector;
    return;
}

void interpreter_destroy(Interpreter interpreter) {
    list_free(interpreter.statements, stmt_free);
    env_destroy(interpreter.environment);
    return;
}

void interpreter_eval(Interpreter* interpreter) {
    l_list_t stmts = interpreter->statements;
    while(stmts) {
        eval_stmt(interpreter, stmts->data);
        stmts = stmts->next;
    }
    return;
}

value_t *eval_stmt(Interpreter *i, Stmt_t* s) {
    value_t *res = return_null(i);
    switch (s->type) {
        case STMT_EXPR:
            res = eval_stmt_exp(i, s);
            break;
        case STMT_PRINT:
            res = eval_stmt_print(i, s);
            break;
        case STMT_IF:
            res = eval_stmt_conditional(i, s);
            break;
        case STMT_BLOCK:
            res = eval_stmt_block(i, s);
            //gc_run(i->garbage_collector);
            break;
        case STMT_DECLARATION:
            res = eval_stmt_declaration(i, s);
            //gc_run(i->garbage_collector);
            break;
        case STMT_ASSIGNMENT:
            res = eval_stmt_assignment(i, s);
            break;
        case STMT_FUN:
            res = eval_stmt_function(i, s);
            //gc_run(i->garbage_collector);
            break;
        default:
            raise_runtime_error(i, "Unimplemented Error\n");
    }
    return res;
}

value_t *eval(Interpreter* i, Exp_t *exp) {
    switch(exp->type) {
        case EXP_UNARY:
            return eval_unary(i, exp);
        case EXP_BINARY:
            return eval_binary(i, exp);
        case EXP_GROUPING:
            return eval_grouping(i, exp);
        case EXP_LITERAL:
            return eval_literal(i, exp);
        case EXP_IDENTIFIER:
            return eval_identifier(i, exp);
        case EXP_CALL:
            return eval_call(i, exp);
        default: // Unreachable
            raise_runtime_error(i, "Unknown expression\n");
    }

    // Unreachable
    return NULL;
}

value_t *eval_literal(Interpreter *i, Exp_t *exp) {
    Exp_literal_t *unwrapped_exp = exp_unwrap(exp);
    switch(unwrapped_exp->type) {
        case T_STRING:
            return gc_init_string(i->garbage_collector, unwrapped_exp->value);
        case T_NUMBER:
            return gc_init_number(i->garbage_collector, *((double*)unwrapped_exp->value));
        case T_BOOLEAN:
            return gc_init_boolean(i->garbage_collector, *((int*)unwrapped_exp->value));
        case T_NIL:
            return gc_init_nil(i->garbage_collector);
        default:
            //unreachable
            return return_null(i);
    }
}

value_t *eval_identifier(Interpreter *i, Exp_t *exp){
    Exp_identifier_t *unwrapped_exp = exp_unwrap(exp);
    value_t *v = env_get(i->environment, unwrapped_exp->identifier);
    if(v == NULL)
        raise_runtime_error(i, "Undeclared identifier\n");
    return v;
}

value_t *eval_grouping(Interpreter* i, Exp_t *exp) {
    Exp_grouping_t *unwrapped_exp = exp_unwrap(exp);
    return eval(i, unwrapped_exp->exp);
}

value_t *eval_unary(Interpreter *i, Exp_t *exp) {
    Exp_unary_t *unwrapped_exp = exp_unwrap(exp);
    value_t *right = eval(i, unwrapped_exp->right);
    switch(unwrapped_exp->op) {
        case OP_MINUS:
            if(right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operand must be a number\n");
            return gc_init_number(i->garbage_collector, -(*(double*)right->value));
        case OP_NOT:
            return gc_init_boolean(i->garbage_collector, !is_truthy(i, right));
        default: // Theoretically unreachable
            raise_runtime_error(i, "Unkown operation\n");
    }

    // Unreachable
    return NULL;
}

value_t *eval_binary(Interpreter *i, Exp_t *exp) {
    Exp_binary_t *unwrapped_exp = exp_unwrap(exp);
    value_t *right = eval(i, unwrapped_exp->right);
    gc_store(i->garbage_collector, right);
    value_t *left = eval(i, unwrapped_exp->left);
    gc_store(i->garbage_collector, left);
    value_t *result = NULL;
    switch (unwrapped_exp->op) {
        case OP_MINUS:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_number(i->garbage_collector, *((double*)left->value) - *((double*)right->value));
            break;
        case OP_STAR:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_number(i->garbage_collector, *((double*)left->value) * *((double*)right->value));
            break;
        case OP_SLASH:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_number(i->garbage_collector, *((double*)left->value) / *((double*)right->value));
            break;
        case OP_PLUS:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_number(i->garbage_collector, *((double*)left->value) + *((double*)right->value));
            break;
        case OP_MOD:
            if(left->type != T_NUMBER || right->type != T_NUMBER){
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            }
            result = gc_init_number(i->garbage_collector, fmod(*((double*)left->value), *((double*)right->value)));
            break;
        case OP_GREATER:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_boolean(i->garbage_collector, *((double*)left->value) > *((double*)right->value));
            break;
        case OP_GREATER_EQUAL:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_boolean(i->garbage_collector, *((double*)left->value) >= *((double*)right->value));
            break;
        case OP_LESS:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_boolean(i->garbage_collector, *((double*)left->value) < *((double*)right->value));
            break;
        case OP_LESS_EQUAL:
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_boolean(i->garbage_collector, *((double*)left->value) <= *((double*)right->value));
            break;
        case OP_EQUAL:
            result = gc_init_boolean(i->garbage_collector, is_equal(i, left, right));
            break;
        case OP_NOT_EQUAL:
            result = gc_init_boolean(i->garbage_collector, !is_equal(i, left, right));
            break;
        default: // Theoretically unreachable
            raise_runtime_error(i, "Unkown Operation\n");
    }
    gc_discard(i->garbage_collector, 2);
    return result;
}

value_t *eval_call(Interpreter *i, Exp_t *exp) {
    Exp_call_t *unwrapped_exp = exp_unwrap(exp);
    l_list_t values = NULL;
    l_list_t expressions = unwrapped_exp->actuals;
    int count = 0;
    while(expressions != NULL) {
        value_t *tmp = eval(i, expressions->data);
        gc_store(i->garbage_collector, tmp);
        list_add(&values, tmp);
        count++;
        expressions = expressions->next;
    }
    list_reverse_in_place(&values);
    value_t *v = env_get(i->environment, unwrapped_exp->identifier);
    if(v->type != T_CLOSURE)
        raise_runtime_error(i, "The identifier '%s' is not a function name\n", unwrapped_exp->identifier);
    closure_t *closure = (closure_t*)v->value;
    int old_size = i->environment->size;
    if(!env_bulk_bind(i->environment, closure->formals, values))
        raise_runtime_error(i, "actuals number and formals number are not the same");
    value_t *res = eval_stmt(i, closure->body);
    env_restore(i->environment, old_size);
    gc_discard(i->garbage_collector, count);
    while(values) {
        l_list_t tmp = values;
        values = values->next;
        mem_free(tmp);
    }
    res->status = 1;
    return res;
}

value_t *eval_stmt_exp(Interpreter *i, Stmt_t *s) {
    Stmt_expr_t *unwrapped_stmt = stmt_unwrap(s);
    return eval(i, unwrapped_stmt->exp);
}

value_t *eval_stmt_print(Interpreter *i, Stmt_t *s) {
    Stmt_print_t *unwrapped_stmt = stmt_unwrap(s);
    value_t *v = eval(i, unwrapped_stmt->exp);
    pretty_print(v);
    return return_null(i); 
}

value_t *eval_stmt_conditional(Interpreter *i, Stmt_t *s) {
    Stmt_conditional_t *unwrapped_stmt = stmt_unwrap(s);
    value_t *cond = eval(i, unwrapped_stmt->condition);
    gc_store(i->garbage_collector, cond);
    value_t *res = return_null(i);
    if(is_truthy(i, cond))
        res = eval_stmt(i, unwrapped_stmt->then_brench);
    else if(unwrapped_stmt->else_brench != NULL)
        res = eval_stmt(i, unwrapped_stmt->else_brench);
    gc_discard(i->garbage_collector, 1);
    return res;
}

value_t *eval_stmt_block(Interpreter *i, Stmt_t *s) {
    Stmt_block_t *unwrapped_stmt = stmt_unwrap(s);
    l_list_t stmts = unwrapped_stmt->statements;
    int old_size = i->environment->size;
    value_t *v = return_null(i);
    int count = 0;
    while(stmts) {
        v = eval_stmt(i, stmts->data);
        gc_store(i->garbage_collector, v);
        count++;
        stmts = stmts->next;
    }
    env_restore(i->environment, old_size);
    gc_discard(i->garbage_collector, count);
    return v;
}

value_t *eval_stmt_declaration(Interpreter *i, Stmt_t *s) {
    Stmt_declaration_t *unwrapped_stmt = stmt_unwrap(s);
    value_t *v = eval(i, unwrapped_stmt->exp);
    env_bind(i->environment, unwrapped_stmt->identifier, v);
    return return_null(i);
}

value_t *eval_stmt_assignment(Interpreter *i, Stmt_t *s) {
    Stmt_assignment_t *unwrapped_stmt = stmt_unwrap(s);
    value_t *res = eval(i, unwrapped_stmt->exp);
    env_set(i->environment, unwrapped_stmt->identifier, res);
    return res;
}

value_t *eval_stmt_function(Interpreter *i, Stmt_t *s) {
    Stmt_function_t *unwrapped_stmt = stmt_unwrap(s);
    closure_t tmp;
    tmp.body = unwrapped_stmt->body;
    tmp.formals = unwrapped_stmt->formals;
    tmp.identifier = unwrapped_stmt->identifier;
    value_t *closure = gc_init_closure(i->garbage_collector, tmp);
    env_bind(i->environment, unwrapped_stmt->identifier, closure);
    return closure;
}

value_t *return_null(Interpreter *i) {
    return gc_init_nil(i->garbage_collector);
}

int is_equal(Interpreter *i, value_t *l, value_t *r) {
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
        case T_CLOSURE:
            raise_runtime_error(i, "Type Error:\t Functions cannot be compared\n");
    }
    return 0;
}

int is_truthy(Interpreter *i, value_t *v) {
    switch(v->type) {
        case T_BOOLEAN:
            return *((int*)v->value);
        default:
            raise_runtime_error(i, "Type Error:\tImplicit casting is not permitted!\n");
            // Unreachable
            return 0;
    }
}

void pretty_print(value_t *v) {
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
            break;
        case T_CLOSURE:
            printf("fun%s\n", ((closure_t*)v->value)->identifier);
            break;
    }
    fflush(stdout);
    return;
}


void bulk_pretty_print(l_list_t values) {
    l_list_t value = values;
    while(value != NULL) {
        pretty_print(value->data);
        value = value->next;
    }
    return;
}

void raise_runtime_error(Interpreter *i, char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    Log_v(ERROR, msg, ap);
    interpreter_destroy(*i);
    va_end(ap);
    exit(EXIT_FAILURE);
}

