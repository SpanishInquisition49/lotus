#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include "garbage.h"
#include "interpreter.h"
#include "list.h"
#include "syntax.h"
#include "memory.h"
#include "errors.h"

static int stack_pointer;
static jmp_buf stack[STACK_SIZE];

/**
 * Evaluate the given expression
 * @param i a pointer to the interpreter
 * @param exp a pointer to the expression to evaluate
 * @return a pointer to the value obtained
 * @note This is a facade for the real evaluation functions
 */
static value_t *eval(interpreter_t*, exp_t*);
/**
 * Evaluate the given literal expression
 * @param i a pointer to the interpreter
 * @param exp a pointer to the expression to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval function
 */
static value_t *eval_literal(interpreter_t *, exp_t*);
/**
 * Evaluate the given grouping expression
 * @param i a pointer to the interpreter
 * @param exp a pointer to the expression to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval function
 */
static value_t *eval_grouping(interpreter_t*, exp_t*);
/**
 * Evaluate the given unary expression
 * @param i a pointer to the interpreter
 * @param exp a pointer to the expression to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval function
 */
static value_t *eval_unary(interpreter_t*, exp_t*);
/**
 * Evaluate the given binary expression
 * @param i a pointer to the interpreter
 * @param exp a pointer to the expression to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval function
 */
static value_t *eval_binary(interpreter_t*, exp_t*);
/**
 * Evaluate the given identifier expression
 * @param i a pointer to the interpreter
 * @param exp a pointer to the expression to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval function
 */
static value_t *eval_identifier(interpreter_t*, exp_t*);
/**
 * Evaluate the given call expression
 * @param i a pointer to the interpreter
 * @param exp a pointer to the expression to evaluate
 * @param forwarded a pointer to a forwarded value to be used as a first argument
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval function
 */
static value_t *eval_call(interpreter_t*, exp_t*, value_t*);
/**
 * Evaluate the given forwarding expression
 * @param i a pointer to the interpreter
 * @param left a pointer to the left side expression
 * @param right a pointer to the right side expression
 * @return a pointer to the value obtained
 * @note Auxiliary function used inside the eval_binary function 
 */
static value_t *eval_forwarding(interpreter_t*, exp_t*, exp_t*);

/**
 * Lazy evaluate left and right side of values of an expression
 * @param i a pointer to the interpreter
 * @param op the type of the operation
 * @param left a pointer to the left side expression
 * @param right a pointer to the right side expression
 * @param left_v a pointer to the left side value
 * @param right_v a pointer to the right side value
 * @return 1 if a short circuit happened, 0 otherwise
 * @note This function do not perform the operation
 */
static int eval_lazy(interpreter_t*, operator_t, exp_t*, exp_t*, value_t**, value_t**);

/**
 * Evaluate the given statement
 * @param i a pointer to the interpreter
 * @param s a pointer to the statement to evaluate
 * @return a pointer to the value obtained
 * @note This is a facade for the real statement evaluation functions
 */
static value_t *eval_stmt(interpreter_t*, stmt_t*);
/**
 * Evaluate the given statement expression
 * @param i a pointer to the interpreter
 * @param stmt a pointer to the statement to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval_stmt function
 */
static value_t *eval_stmt_exp(interpreter_t*, stmt_t*);
/**
 * Evaluate the given statement print
 * @param i a pointer to the interpreter
 * @param stmt a pointer to the statement to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval_stmt function
 */
static value_t *eval_stmt_print(interpreter_t*, stmt_t*);
/**
 * Evaluate the given statement conditional
 * @param i a pointer to the interpreter
 * @param stmt a pointer to the statement to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval_stmt function
 */
static value_t *eval_stmt_conditional(interpreter_t*, stmt_t*);
/**
 * Evaluate the given statement block
 * @param i a pointer to the interpreter
 * @param stmt a pointer to the statement to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval_stmt function
 */
static value_t *eval_stmt_block(interpreter_t*, stmt_t*);
/**
 * Evaluate the given statement expression
 * @param i a pointer to the interpreter
 * @param stmt a pointer to the statement to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval_stmt function
 */
static value_t *eval_stmt_declaration(interpreter_t*, stmt_t*);
/**
 * Evaluate the given statement assignment
 * @param i a pointer to the interpreter
 * @param stmt a pointer to the statement to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval_stmt function
 */
static value_t *eval_stmt_assignment(interpreter_t*, stmt_t*);
/**
 * Evaluate the given statement function declaration
 * @param i a pointer to the interpreter
 * @param stmt a pointer to the statement to evaluate
 * @return a pointer to the value obtained 
 * @note Auxiliary function used inside the main eval_stmt function
 */
static value_t *eval_stmt_function(interpreter_t*, stmt_t*);

/**
 * @note Utility function
 * @return Nil in the Lotus Language 
 */
static value_t *return_null(interpreter_t*);
/**
 * Concatenate two strings in the Lotus Language
 * @return a value pointer to the new string
 * @note Utility function 
 */
static value_t *str_concat(interpreter_t*, value_t*, value_t*);
/**
 * Check if the two given value in the Lotus Language are equal
 * @param i a pointer to the interpreter
 * @param l a pointer to the left side value
 * @param r a pointer to the right side value
 * @return 1 if they are they are equal, 0 otherwise
 * @note Utility function
 */
static int is_equal(interpreter_t*, value_t*, value_t*);
/**
 * Check if the given value is truthy
 * @param i a pointer to the interpreter
 * @param v a pointer to the value 
 * @note Utility function
 */
static int is_truthy(interpreter_t*, value_t*);
/**
 * Pretty print a list of values
 * @param values a list of values to be printed 
 * @note Utility function
 */
static void bulk_pretty_print(l_list_t);
/**
 * Pretty print a value
 * @param v a pointer to the value to be printed 
 * @note Utility function
 */
static void pretty_print(value_t*);
/**
 * Stop the execution and show a formatted error message
 * @param i a pointer to the interpreter
 * @param msg the message to be printed
 */
__attribute__((noreturn))
__attribute__((format (printf, 2, 3)))
static void raise_runtime_error(interpreter_t*, char*, ...);

void interpreter_init(interpreter_t *interpreter, env_t *env,l_list_t statements, garbage_collector_t *garbage_collector) {
    memset(interpreter, 0, sizeof(interpreter_t));
    interpreter->statements = statements;
    interpreter->environment = env;
    interpreter->garbage_collector = garbage_collector;
    interpreter->returned_value = NULL;
    stack_pointer = 0;
    memset(stack, 0, sizeof(jmp_buf) * STACK_SIZE);
    return;
}

void interpreter_destroy(interpreter_t interpreter) {
    list_free(interpreter.statements, stmt_free);
    env_destroy(interpreter.environment);
    return;
}

void interpreter_eval(interpreter_t* interpreter) {
    l_list_t stmts = interpreter->statements;
    while(stmts) {
        eval_stmt(interpreter, stmts->data);
        stmts = stmts->next;
    }
    return;
}

value_t *eval_stmt(interpreter_t *i, stmt_t* s) {
    switch (s->type) {
        case STMT_RETURN:
            if(stack_pointer - 1 < 0)
                raise_runtime_error(i, "return can used only inside a function\n");
            i->returned_value = eval_stmt_exp(i, s);
            longjmp(stack[stack_pointer - 1], 1);
        case STMT_EXPR:
            return eval_stmt_exp(i, s);
            break;
        case STMT_PRINT:
            return eval_stmt_print(i, s);
            break;
        case STMT_IF:
            return eval_stmt_conditional(i, s);
            break;
        case STMT_BLOCK:
            return eval_stmt_block(i, s);
            //gc_run(i->garbage_collector);
            break;
        case STMT_DECLARATION:
            return eval_stmt_declaration(i, s);
            //gc_run(i->garbage_collector);
            break;
        case STMT_ASSIGNMENT:
            return eval_stmt_assignment(i, s);
            break;
        case STMT_FUN:
            return eval_stmt_function(i, s);
            //gc_run(i->garbage_collector);
            break;
        default:
            raise_runtime_error(i, "Unimplemented Error\n");
    }
}

value_t *eval(interpreter_t* i, exp_t *exp) {
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
            return eval_call(i, exp, NULL);
        default:
            raise_runtime_error(i, "Unknown expression\n");
    }
}

value_t *eval_literal(interpreter_t *i, exp_t *exp) {
    exp_literal_t *unwrapped_exp = exp_unwrap(exp);
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
            __builtin_unreachable();
    }
}

value_t *eval_identifier(interpreter_t *i, exp_t *exp){
    exp_identifier_t *unwrapped_exp = exp_unwrap(exp);
    value_t *v = env_get(i->environment, unwrapped_exp->identifier);
    if(v == NULL)
        raise_runtime_error(i, "The identifier '%s' was not declared\n", unwrapped_exp->identifier);
    return v;
}

value_t *eval_grouping(interpreter_t* i, exp_t *exp) {
    exp_grouping_t *unwrapped_exp = exp_unwrap(exp);
    return eval(i, unwrapped_exp->exp);
}

value_t *eval_unary(interpreter_t *i, exp_t *exp) {
    exp_unary_t *unwrapped_exp = exp_unwrap(exp);
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
}

value_t *eval_binary(interpreter_t *i, exp_t *exp) {
    exp_binary_t *unwrapped_exp = exp_unwrap(exp);
    int short_circuit = 0;
    value_t *right = NULL;
    value_t *left = NULL;
    value_t *result = NULL;
    switch (unwrapped_exp->op) {
        case OP_FORWARD:
            return eval_forwarding(i, unwrapped_exp->left, unwrapped_exp->right);
        case OP_MINUS:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_number(i->garbage_collector, *((double*)left->value) - *((double*)right->value));
            break;
        case OP_STAR:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_number(i->garbage_collector, *((double*)left->value) * *((double*)right->value));
            break;
        case OP_SLASH:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_number(i->garbage_collector, *((double*)left->value) / *((double*)right->value));
            break;
        case OP_PLUS:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            if(left->type == T_NUMBER && right->type == T_NUMBER)
                result = gc_init_number(i->garbage_collector, *((double*)left->value) + *((double*)right->value));
            else if(left->type == T_STRING && right->type == T_STRING)
                result = str_concat(i, left, right);
            else
                raise_runtime_error(i, "Type Error:\t Operands must be two numbers or two strings\n");
            break;
        case OP_MOD:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            if(left->type != T_NUMBER || right->type != T_NUMBER){
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            }
            result = gc_init_number(i->garbage_collector, fmod(*((double*)left->value), *((double*)right->value)));
            break;
        case OP_GREATER:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_boolean(i->garbage_collector, *((double*)left->value) > *((double*)right->value));
            break;
        case OP_GREATER_EQUAL:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_boolean(i->garbage_collector, *((double*)left->value) >= *((double*)right->value));
            break;
        case OP_LESS:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_boolean(i->garbage_collector, *((double*)left->value) < *((double*)right->value));
            break;
        case OP_LESS_EQUAL:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            if(left->type != T_NUMBER || right->type != T_NUMBER)
                raise_runtime_error(i, "Type Error:\t Operands must be numbers\n");
            result = gc_init_boolean(i->garbage_collector, *((double*)left->value) <= *((double*)right->value));
            break;
        case OP_EQUAL:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            result = gc_init_boolean(i->garbage_collector, is_equal(i, left, right));
            break;
        case OP_NOT_EQUAL:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            result = gc_init_boolean(i->garbage_collector, !is_equal(i, left, right));
            break;
        case OP_AND:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            result = gc_init_boolean(i->garbage_collector, *((int*)left->value) && *((int*)right->value));
            break;
        case OP_OR:
            short_circuit = eval_lazy(i, unwrapped_exp->op, unwrapped_exp->left, unwrapped_exp->right, &left, &right);
            result = gc_init_boolean(i->garbage_collector, *((int*)left->value) || *((int*)right->value));
            break; 
        default: // Theoretically unreachable
            raise_runtime_error(i, "Unkown Operation\n");
    }
    gc_release(i->garbage_collector, short_circuit ? 1 : 2);
    return result;
}

int eval_lazy(interpreter_t *i, operator_t op, exp_t *left, exp_t *right, value_t **left_v, value_t **right_v) { 
    *left_v = eval(i, left);
    gc_hold(i->garbage_collector, *left_v);
    switch(op){
        case OP_AND:
            if((*left_v)->type != T_BOOLEAN)
                raise_runtime_error(i, "Type Error:\t Operands must be booleans\n");
            if(*((int*)(*left_v)->value) == 0)
                return 1;
            break;
        case OP_OR:
            if((*left_v)->type != T_BOOLEAN)
                raise_runtime_error(i, "Type Error:\t Operands must be booleans\n");
            if(*((int*)(*left_v)->value) == 1)
                return 1;
            break;
        default:
            break;
    }
    *right_v = eval(i, right);
    if((op == OP_AND || op == OP_OR) && (*right_v)->type != T_BOOLEAN)
        raise_runtime_error(i, "Type Error:\t Operands must be booleans\n");
    gc_hold(i->garbage_collector, *right_v);
    return 0;
}

value_t *eval_forwarding(interpreter_t *i, exp_t *left, exp_t *right) {
    if(right->type != EXP_CALL)
        raise_runtime_error(i, "Expected a function call after |>\n");
    value_t *left_v = eval(i, left);
    gc_hold(i->garbage_collector, left_v);
    value_t *res = eval_call(i, right, left_v);
    gc_release(i->garbage_collector, 1);
    return res;
}

value_t *eval_call(interpreter_t *i, exp_t *exp, value_t* forwarded) {
    exp_call_t *unwrapped_exp = exp_unwrap(exp);
    // Evaluating all actuals parameters
    l_list_t values = NULL;
    l_list_t expressions = unwrapped_exp->actuals;
    int count = 0;
    while(expressions != NULL) {
        value_t *tmp = eval(i, expressions->data);
        gc_hold(i->garbage_collector, tmp);
        list_add(&values, tmp);
        count++;
        expressions = expressions->next;
    }
    if(forwarded)
        list_add(&values, forwarded);
    list_reverse_in_place(&values);
    // Get the closure from the environment
    value_t *v = env_get(i->environment, unwrapped_exp->identifier);
    if(v == NULL)
        raise_runtime_error(i, "The identifier '%s' was not declared\n", unwrapped_exp->identifier);
    if(v->type != T_CLOSURE)
        raise_runtime_error(i, "The identifier '%s' is not a function name\n", unwrapped_exp->identifier);
    closure_t *closure = (closure_t*)v->value;
    // Saving the current size of the environment
    int old_size = i->environment->size;
    if(!env_bulk_bind(i->environment, closure->formals, values))
        raise_runtime_error(i, "actuals number and formals number are not the same");
    // Releasing the actuals (now they are reachable from the env)
    gc_release(i->garbage_collector, count);
    // Saving the stack pointer preparing for the long jump (return)
    int old_sp = stack_pointer;
    if(stack_pointer >= STACK_SIZE)
        raise_runtime_error(i, "Stack overflow\n");
    int jmp = setjmp(stack[stack_pointer++]);
    // Check if a jmp (return) has happened and set the return value properly
    value_t *res = jmp ? i->returned_value : eval_stmt(i, closure->body);
    // Restoring the environment, stack pointer and cleaning memory
    env_restore(i->environment, old_size);
    while(values) {
        l_list_t tmp = values;
        values = values->next;
        mem_free(tmp);
    }
    stack_pointer = old_sp;
    res->status = 1;
    return res;
}

value_t *eval_stmt_exp(interpreter_t *i, stmt_t *s) {
    stmt_expr_t *unwrapped_stmt = stmt_unwrap(s);
    return eval(i, unwrapped_stmt->exp);
}

value_t *eval_stmt_print(interpreter_t *i, stmt_t *s) {
    stmt_print_t *unwrapped_stmt = stmt_unwrap(s);
    value_t *v = eval(i, unwrapped_stmt->exp);
    pretty_print(v);
    return return_null(i); 
}

value_t *eval_stmt_conditional(interpreter_t *i, stmt_t *s) {
    stmt_conditional_t *unwrapped_stmt = stmt_unwrap(s);
    value_t *cond = eval(i, unwrapped_stmt->condition);
    gc_hold(i->garbage_collector, cond);
    value_t *res = return_null(i);
    if(is_truthy(i, cond))
        res = eval_stmt(i, unwrapped_stmt->then_branch);
    else if(unwrapped_stmt->else_branch != NULL)
        res = eval_stmt(i, unwrapped_stmt->else_branch);
    gc_release(i->garbage_collector, 1);
    return res;
}

value_t *eval_stmt_block(interpreter_t *i, stmt_t *s) {
    stmt_block_t *unwrapped_stmt = stmt_unwrap(s);
    l_list_t stmts = unwrapped_stmt->statements;
    int old_size = i->environment->size;
    value_t *v = return_null(i);
    int count = 0;
    while(stmts) {
        stmt_t *stmt = (stmt_t*)stmts->data;
        v = eval_stmt(i, stmt);
        gc_hold(i->garbage_collector, v);
        count++;
        stmts = stmts->next;
    }
    env_restore(i->environment, old_size);
    gc_release(i->garbage_collector, count);
    return v;
}

value_t *eval_stmt_declaration(interpreter_t *i, stmt_t *s) {
    stmt_declaration_t *unwrapped_stmt = stmt_unwrap(s);
    value_t *v = eval(i, unwrapped_stmt->exp);
    env_bind(i->environment, unwrapped_stmt->identifier, v);
    return v;
}

value_t *eval_stmt_assignment(interpreter_t *i, stmt_t *s) {
    stmt_assignment_t *unwrapped_stmt = stmt_unwrap(s);
    value_t *res = eval(i, unwrapped_stmt->exp);
    env_set(i->environment, unwrapped_stmt->identifier, res);
    return res;
}

value_t *eval_stmt_function(interpreter_t *i, stmt_t *s) {
    stmt_function_t *unwrapped_stmt = stmt_unwrap(s);
    closure_t tmp;
    tmp.body = unwrapped_stmt->body;
    tmp.formals = unwrapped_stmt->formals;
    tmp.identifier = unwrapped_stmt->identifier;
    value_t *closure = gc_init_closure(i->garbage_collector, tmp);
    env_bind(i->environment, unwrapped_stmt->identifier, closure);
    return closure;
}

value_t *return_null(interpreter_t *i) {
    return gc_init_nil(i->garbage_collector);
}

int is_equal(interpreter_t *i, value_t *l, value_t *r) {
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
    __builtin_unreachable();
}

int is_truthy(interpreter_t *i, value_t *v) {
    switch(v->type) {
        case T_BOOLEAN:
            return *((int*)v->value);
        default:
            raise_runtime_error(i, "Type Error:\tImplicit casting is not permitted!\n");
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

value_t *str_concat(interpreter_t *i, value_t *left, value_t *right) {
    char *l = strdup((char*)left->value);
    char *r = strdup((char*)right->value);
    value_t *res = gc_init_string(i->garbage_collector, strcat(l, r));
    mem_free(l);
    mem_free(r);
    return res;
}

void bulk_pretty_print(l_list_t values) {
    l_list_t value = values;
    while(value != NULL) {
        pretty_print(value->data);
        value = value->next;
    }
    return;
}

void raise_runtime_error(interpreter_t *i, char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    err_log_v(ERROR, msg, ap);
    interpreter_destroy(*i);
    va_end(ap);
    exit(EXIT_FAILURE);
}
