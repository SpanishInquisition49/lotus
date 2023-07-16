#ifndef SCANNER_H
#define SCANNER_H
#include "list.h"
#include "token.h"
#include "errors.h"

/**
 * @brief A scanner_t "object"
 * @param const char *filename: The path of the source file to scan
 * @param char* source: The source code inside the filename
 * @param List tokens: The tokens obtained from the lexical analysis
 * @param int start: The starting index of the current token
 * @param int current: The current position of the scanner
 * @param int length: The length (bytes) of the source code
 * @param int errors[]: An array that keep count of various error types
 */
typedef struct { 
    const char* filename;
    char* source;
    l_list_t tokens;
    int line_number;
    int start;
    int current;
    int length;
    int errors[LOG_LEVELS];
} scanner_t;


/**
 * @brief Initialize the given scanner
 * @param scanner_t *scanner: The scanner to initialize
 * @param const char* fil_name: The path of the source file
 */
void scanner_init(scanner_t*, const char*);

/**
 * @brief Destroy the given scanner
 * @param scanner_t scanner: The scanner to destroy
 */
void scanner_destroy(scanner_t);

/**
 * @brief Perform a lexical analysis of the given source file
 * @param scanner_t: The scanner that contains the source filename and the token list
 */ 
void scanner_scan_tokens(scanner_t*);

/**
 * @brief Print a report of the errors and warning in the scanner phase
 * @param scanner_t scanner: The scanner used for the report
 */
void scanner_errors_report(scanner_t);

/**
 * @brief Check if the given scanner have one or more errors of type ERROR
 * @param scanner_t scanner: The scanner to check
 * @return true if an error has occurred in the scanner phase
 */
int scanner_had_error(scanner_t);
#endif // !SCANNER_H
