#ifndef TOKEN_H
#define TOKEN_H
#include "list.h"

typedef enum {
  // Single-character tokens.
  LEFT_PAREN = 1,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  LEFT_SQUARE_BRACKET,
  RIGHT_SQUARE_BRACKET,
  COMMA,
  DOT,
  PLUS,
  MOD,
  SEMICOLON,
  COLON,
  SLASH,
  STAR,
  WILDCARD,
  // One or two character tokens.
  PIPE,
  PIPE_GREATER,
  MINUS,
  SINGLE_ARROW,
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  DOUBLE_ARROW,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,
  // Keywords.
  AND,
  OR,
  IF,
  ELSE,
  FUN,
  NIL,
  PRINT,
  RETURN,
  VAR,
  TRUE,
  FALSE,
  MATCH,
  WITH,
  END,
} token_type_t;

/**
 * @brief A lexical token
 * @param char *lexeme: The sequence of characters that match the pattern for a
 * token
 * @param void *literal: Numerical, logical or textual values
 * @param int line: The line number in the source code
 */
typedef struct {
  token_type_t type;
  char *lexeme;
  void *literal;
  int line;
} token_t;

/**
 * @brief Print the given token to the standard error
 * @param token_t: the token to print
 */
void token_print(token_t);

/**
 * @brief Print the given token list
 * @param List: the token list to print
 */
void tokens_print(l_list_t);

/**
 * @brief duplicate a given list of tokens inside another
 * @param List source: The list to duplicate
 * @param List *destination: The list* where to duplicate
 */
void tokens_dup(l_list_t, l_list_t *);

/**
 * @brief Get the token at the given position from the given list
 * @param List tokens: the token list
 * @param int position: the position to retrieve
 * @return token_t* a token pointer if position < list_len(tokens) or NULL
 */
token_t *tokens_get(l_list_t, int);

/**
 * @brief Clear all the allocated data for a token
 * @param void* token: the token to deallocate
 * @note the token is void* because the list_free need a void* free callback
 */
void token_free(void *);

#endif // !TOKEN_H
