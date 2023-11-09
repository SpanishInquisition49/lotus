#include "token.h"
#include "errors.h"
#include "list.h"
#include "memory.h"
#include <stdio.h>
#include <string.h>

static char *pretty_type(token_type_t);

void token_free(void *token) {
  token_t *t = (token_t *)token;
  if (t->type != END)
    mem_free(t->lexeme);
  mem_free(t->literal);
  mem_free(token);
  return;
}

void token_print(token_t tok) {
  err_log(INFO, "Line:  %d\t\tLexeme: %s\t\t\tToken Type: %s\n", tok.line,
          tok.lexeme, pretty_type(tok.type));
  return;
}

void tokens_print(l_list_t tokens) {
  l_list_t current = list_reverse(tokens);
  while (current) {
    token_print(*(token_t *)current->data);
    current = current->next;
  }
  return;
}

token_t *tokens_get(l_list_t tokens, int index) {
  if (index >= list_len(tokens))
    return NULL;
  l_list_t current = tokens;
  for (int i = 0; i < index; i++) {
    current = current->next;
  }
  return current->data;
}

void tokens_dup(l_list_t source, l_list_t *dest) {
  l_list_t head = source;
  while (head) {
    token_t *duped = mem_calloc(1, sizeof(token_t));
    token_t *tok = (token_t *)head->data;
    duped->line = tok->line;
    duped->type = tok->type;
    duped->lexeme = tok->type != END ? strdup(tok->lexeme) : "";
    duped->literal = tok->literal ? strdup(tok->literal) : NULL;
    list_add(dest, duped);
    head = head->next;
  }
}

char *pretty_type(token_type_t type) {
  switch (type) {
  case LEFT_PAREN:
    return "LEFT_PAREN";
  case RIGHT_PAREN:
    return "RIGHT_PAREN";
  case LEFT_BRACE:
    return "LEFT_BRACE";
  case RIGHT_BRACE:
    return "RIGHT_BRACE";
  case LEFT_SQUARE_BRACKET:
    return "LEFT_SQUARE_BRACKET";
  case RIGHT_SQUARE_BRACKET:
    return "RIGHT_SQUARE_BRACKET";
  case COMMA:
    return "COMMA";
  case DOT:
    return "DOT";
  case MINUS:
    return "MINUS";
  case PLUS:
    return "PLUS";
  case MOD:
    return "MODULE";
  case SEMICOLON:
    return "SEMICOLON";
  case COLON:
    return "COLON";
  case SLASH:
    return "SLASH";
  case STAR:
    return "STAR";
  case BANG:
    return "BANG";
  case BANG_EQUAL:
    return "BANG_EQUAL";
  case EQUAL:
    return "EQUAL";
  case EQUAL_EQUAL:
    return "EQUAL_EQUAL";
  case GREATER:
    return "GREATER";
  case GREATER_EQUAL:
    return "GREATER_EQUAL";
  case LESS:
    return "LESS";
  case LESS_EQUAL:
    return "LESS_EQUAL";
  case IDENTIFIER:
    return "IDENTIFIER";
  case STRING:
    return "STRING";
  case NUMBER:
    return "NUMBER";
  case AND:
    return "AND";
  case OR:
    return "OR";
  case IF:
    return "IF";
  case ELSE:
    return "ELSE";
  case FUN:
    return "FUN";
  case NIL:
    return "NIL";
  case PRINT:
    return "PRINT";
  case RETURN:
    return "RETURN";
  case VAR:
    return "VAR";
  case TRUE:
    return "TRUE";
  case FALSE:
    return "FALSE";
  case END:
    return "END";
  case SINGLE_ARROW:
    return "SINGLE_ARROW";
  case DOUBLE_ARROW:
    return "DOUBLE_ARROW";
  case PIPE:
    return "PIPE";
  case PIPE_GREATER:
    return "PIPE_GREATER";
  case MATCH:
    return "MATCH";
  case WITH:
    return "WITH";
  case WILDCARD:
    return "WILDCARD";
  default:
    return "UNKOWN";
  }
}
