#include "token.h"
#include "errors.h"
#include "list.h"
#include "memory.h"
#include <stdio.h>
#include <string.h>

static char* pretty_type(TokenType);

void token_free(void *token) {
    Token *t = (Token*)token;
    if(t->lexeme && t->type != END)
        free(t->lexeme);
    free(t->literal);
    free(token);
    return;
}

void token_print(Token tok) {
    Log(INFO, "Line:  %d\t\tLexeme: %s\t\t\tToken Type: %s\n", tok.line, tok.lexeme,  pretty_type(tok.type));
    return;
}

void tokens_print(List tokens) {
    List current = list_reverse(tokens);
    while(current) {
        token_print(*(Token*)current->data);
        current = current->next;
    }
    return;
}

Token *tokens_get(List tokens, int index) {
    if(index >= list_len(tokens))
        return NULL;
    List current = tokens;
    for(int i = 0; i< index; i++){
        current = current->next;
    }
    return current->data;
}

void tokens_dup(List source, List *dest) {
    List head = source;
    while(head) {
        Token *duped = mem_calloc(1, sizeof(Token));
        Token *tok = (Token*)head->data;
        duped->line = tok->line;
        duped->type = tok->type;
        duped->lexeme = tok->type != END ? strdup(tok->lexeme) : "";
        duped->literal = tok->literal ? strdup(tok->literal) : NULL;
        list_add(dest, duped);
        head = head->next;
    }
}

char *pretty_type(TokenType type) {
    switch(type) {
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
