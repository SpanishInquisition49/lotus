#ifndef SYNTAX_H
#define SYNTAX_H



enum ExpType {
    UNARY,
    BINARY,
};

/**
 * This type act as a wrapper around all the possible expression type
 * @note: proprer casting is required
 */
typedef struct {
    enum ExpType type;
    void *exp;
} Exp_t;

typedef struct {
    // TODO
} Token_t;

typedef struct {
    void *exp;
    Token_t *op;
} Exp_unary_t;

typedef struct {
    void *left;
    Token_t *op;
    void *right;
} Exp_binary_t;

#endif // !SYNTAX_H
