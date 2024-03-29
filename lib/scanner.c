#include "scanner.h"
#include "errors.h"
#include "keywords.h"
#include "list.h"
#include "memory.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void scan_token(scanner_t *, char);
static void add_token(scanner_t *, token_type_t, char *);
static char advance(scanner_t *);
static char peek(scanner_t *);
static char peek_next(scanner_t *);
static int match(scanner_t *, char);
static void string(scanner_t *);
static void number(scanner_t *);
static void identifier(scanner_t *);
static int is_at_end(scanner_t *);
static int is_alpha(char);
static int is_digit(char);
static int is_alphanumeric(char);
static int keyword_get(char *);

void scanner_init(scanner_t *scanner, const char *file_name) {
  memset(scanner, 0, sizeof(*scanner));
  scanner->current = 0;
  scanner->start = 0;
  scanner->length = 0;
  scanner->line_number = 1;
  scanner->tokens = NULL;
  scanner->source = NULL;
  scanner->filename = file_name;
  for (int i = 0; i < LOG_LEVELS; i++)
    scanner->errors[i] = 0;
  return;
}

void scanner_destroy(scanner_t scanner) {
  mem_free(scanner.source);
  list_free(scanner.tokens, token_free);
  return;
}

void scanner_errors_report(scanner_t scanner) {
  dprintf(2, "%s[SCANNER]\t%sErrors: %d\t%sWarnings: %d%s\n",
          ANSI_COLOR_MAGENTA, ANSI_COLOR_RED, scanner.errors[ERROR],
          ANSI_COLOR_YELLOW, scanner.errors[WARNING], ANSI_COLOR_RESET);
  return;
}

int scanner_had_error(scanner_t scanner) { return scanner.errors[ERROR]; }

void scanner_scan_tokens(scanner_t *scanner) {
  FILE *f = fopen(scanner->filename, "r");
  if (f == NULL) {
    scanner->errors[ERROR]++;
    err_log(ERROR, "File '%s' is not a valid file\n", scanner->source);
    exit(EXIT_FAILURE);
  }
  fseek(f, 0, SEEK_END);
  scanner->length = ftell(f);
  fseek(f, 0, SEEK_SET);
  scanner->source = malloc(sizeof(char) * scanner->length);
  if (scanner->source == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  fread(scanner->source, 1, scanner->length, f);
  fclose(f);
  while (!is_at_end(scanner)) {
    scanner->start = scanner->current;
    char c = advance(scanner);
    scan_token(scanner, c);
  }
  token_t *eof = mem_calloc(1, sizeof(token_t));
  eof->type = END;
  eof->lexeme = "";
  eof->literal = NULL;
  eof->line = scanner->line_number;
  list_add(&scanner->tokens, eof);
  token_print(*eof);
}

void scan_token(scanner_t *scanner, char c) {
  switch (c) {
  case K_LEFT_PAREN:
    add_token(scanner, LEFT_PAREN, NULL);
    break;
  case K_RIGHT_PAREN:
    add_token(scanner, RIGHT_PAREN, NULL);
    break;
  case K_LEFT_BRACE:
    add_token(scanner, LEFT_BRACE, NULL);
    break;
  case K_RIGHT_BRACE:
    add_token(scanner, RIGHT_BRACE, NULL);
    break;
  case K_LEFT_SQUARE_BRACKET:
    add_token(scanner, LEFT_SQUARE_BRACKET, NULL);
    break;
  case K_RIGHT_SQUARE_BRACKET:
    add_token(scanner, RIGHT_SQUARE_BRACKET, NULL);
    break;
  case K_COMMA:
    add_token(scanner, COMMA, NULL);
    break;
  case K_DOT:
    add_token(scanner, DOT, NULL);
    break;
  case K_MINUS:
    add_token(scanner, match(scanner, K_GREATER) ? SINGLE_ARROW : MINUS, NULL);
    break;
  case K_PLUS:
    add_token(scanner, PLUS, NULL);
    break;
  case K_STAR:
    add_token(scanner, STAR, NULL);
    break;
  case K_MOD:
    add_token(scanner, MOD, NULL);
    break;
  case K_PIPE:
    add_token(scanner, match(scanner, K_GREATER) ? PIPE_GREATER : PIPE, NULL);
    break;
  case K_SEMICOLON:
    add_token(scanner, SEMICOLON, NULL);
    break;
  case K_COLON:
    add_token(scanner, COLON, NULL);
    break;
  case K_BANG:
    add_token(scanner, match(scanner, K_EQUAL) ? BANG_EQUAL : BANG, NULL);
    break;
  case K_EQUAL:
    add_token(scanner,
              match(scanner, K_EQUAL)     ? EQUAL_EQUAL
              : match(scanner, K_GREATER) ? DOUBLE_ARROW
                                          : EQUAL,
              NULL);
    break;
  case K_LOWER:
    add_token(scanner, match(scanner, K_EQUAL) ? LESS_EQUAL : LESS, NULL);
    break;
  case K_GREATER:
    add_token(scanner, match(scanner, K_EQUAL) ? GREATER_EQUAL : GREATER, NULL);
    break;
  case K_SLASH:
    if (match(scanner, K_SLASH)) {
      // Ignore the comment until the \n
      while (peek(scanner) != '\n' && !is_at_end(scanner))
        advance(scanner);
    } else {
      add_token(scanner, SLASH, NULL);
    }
    break;
  case K_DOUBLE_QUOTE:
    string(scanner);
    break;
  case ' ':
  case '\t':
  case '\r':
    // Ignore whitespaces
    break;
  case '\n':
    scanner->line_number++;
    break;
  default:
    if (is_digit(c))
      number(scanner);
    // TODO: Keep a watch on this condition (is_alphanumeric should be negated)
    else if (c == K_WILDCARD && is_alphanumeric(peek_next(scanner)))
      add_token(scanner, WILDCARD, NULL);
    else if (is_alpha(c))
      identifier(scanner);
    else {
      scanner->errors[WARNING]++;
      err_log(WARNING, "Line %d: Unknown character: '%c'\n",
              scanner->line_number, c);
    }
    break;
  }
  return;
}

int is_at_end(scanner_t *s) { return s->current >= s->length; }

int is_digit(char c) { return c >= '0' && c <= '9'; }

int is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int is_alphanumeric(char c) { return is_alpha(c) || is_digit(c); }

int match(scanner_t *s, char expected) {
  if (is_at_end(s))
    return 0;
  if (peek(s) != expected)
    return 0;
  s->current++;
  return 1;
}

char advance(scanner_t *s) { return s->source[s->current++]; }

char peek(scanner_t *s) {
  if (is_at_end(s))
    return '\0';
  return s->source[s->current];
}

char peek_next(scanner_t *s) {
  if (s->current + 1 >= s->length)
    return '\0';
  return s->source[s->current + 1];
}

void string(scanner_t *s) {
  int line_start = s->line_number;
  // NOTE: special char escape should occur here
  while (peek(s) != K_DOUBLE_QUOTE && !is_at_end(s)) {
    if (peek(s) == '\n')
      s->line_number++;
    advance(s);
  }

  if (is_at_end(s)) {
    s->errors[ERROR]++;
    err_log(ERROR, "Line %d: Missing closing \"\n", line_start);
    return;
  }
  // advance in order to consume '"' character
  advance(s);
  int len = s->current - s->start;
  add_token(s, STRING, strndup(s->source + s->start + 1, len - 2));
}

void number(scanner_t *s) {
  while (is_digit(peek(s)))
    advance(s);

  // Look for a fractional part
  if (peek(s) == K_DOT && is_digit(peek_next(s))) {
    advance(s);
    while (is_digit(peek(s)))
      advance(s);
  } else if (peek(s) == K_DOT && !is_digit(peek_next(s))) {
    s->errors[ERROR]++;
    err_log(ERROR, "Line %d: Wrong format for number\n", s->line_number);
    return;
  }
  int len = s->current - s->start;
  add_token(s, NUMBER, strndup(s->source + s->start, len));
}

void identifier(scanner_t *s) {
  while (is_alphanumeric(peek(s)))
    advance(s);
  int len = s->current - s->start;
  char *text = strndup(s->source + s->start, len);
  add_token(s, keyword_get(text), text);
}

void add_token(scanner_t *s, token_type_t t, char *literal) {
  int len = s->current - s->start;
  token_t *tok = mem_calloc(1, sizeof(token_t));
  tok->lexeme = strndup(s->source + s->start, len);
  tok->literal = literal;
  tok->type = t;
  tok->line = s->line_number;
  list_add(&s->tokens, tok);
  token_print(*tok);
}

int keyword_get(char *text) {
  if (strcmp(text, K_AND) == 0)
    return AND;
  if (strcmp(text, K_ELSE) == 0)
    return ELSE;
  if (strcmp(text, K_FALSE) == 0)
    return FALSE;
  if (strcmp(text, K_FUN) == 0)
    return FUN;
  if (strcmp(text, K_IF) == 0)
    return IF;
  if (strcmp(text, K_NIL) == 0)
    return NIL;
  if (strcmp(text, K_OR) == 0)
    return OR;
  if (strcmp(text, K_PRINT) == 0)
    return PRINT;
  if (strcmp(text, K_RETURN) == 0)
    return RETURN;
  if (strcmp(text, K_TRUE) == 0)
    return TRUE;
  if (strcmp(text, K_VAR) == 0)
    return VAR;
  if (strcmp(text, K_MATCH) == 0)
    return MATCH;
  if (strcmp(text, K_WITH) == 0)
    return WITH;

  return IDENTIFIER;
}
