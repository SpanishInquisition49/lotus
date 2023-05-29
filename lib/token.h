#ifndef TOKEN_H
#define TOKEN_H
#include "list.h"

enum TokenType {
    // Single-character tokens.
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_SQUARE_BRACKET,
    RIGHT_SQUARE_BRACKET,
    COMMA,
    DOT,
    PLUS,
    SEMICOLON,
    COLON,
    SLASH,
    STAR,
    PIPE,
    // One or two character tokens.
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
};

typedef struct {
    enum TokenType type;
    char* lexeme;
    void* literal;
    int line;
} Token;

/**
 * Print the given token to the standard error
 * @param Token: the token to print
 */
void token_print(Token);

/**
 * Print the given token list
 * @param List: the token list to print
 */
void tokens_print(List);

void tokens_dup(List, List*);

#endif // !TOKEN_HTOKEN_H
