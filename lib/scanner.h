#ifndef SCANNER_H
#define SCANNER_H
#include "list.h"
#include "token.h"
#include "errors.h"

/**
 * @brief A Scanner "object"
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
    List tokens;
    int line_number;
    int start;
    int current;
    int length;
    int errors[LOG_LEVELS];
} Scanner;


/**
 * @brief Initialize the given scanner
 * @param Scanner *scanner: The scanner to initialize
 * @param const char* fil_name: The path of the source file
 */
void scanner_init(Scanner*, const char*);

/**
 * @brief Destroy the given scanner
 * @param Scanner scanner: The scanner to destroy
 */
void scanner_destroy(Scanner);

/**
 * @brief Perform a lexical analysis of the given source file
 * @param Scanner: The scanner that contains the source filename and the token list
 */ 
void scanner_scan_tokens(Scanner*);

/**
 * @brief Print a report of the errors and warning in the scanner phase
 * @param Scanner scanner: The scanner used for the report
 */
void scanner_errors_report(Scanner);

/**
 * @brief Check if the given scanner have one or more errors of type ERROR
 * @param Scanner scanner: The scanner to check
 * @return true if an error has occurred in the scanner phase
 */
int scanner_had_error(Scanner);
#endif // !SCANNER_H
