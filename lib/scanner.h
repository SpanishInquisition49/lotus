#ifndef SCANNER_H
#define SCANNER_H
#include "list.h"
#include "token.h"
#include "errors.h"

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
 * Initialize the given scanner
 */
void scanner_init(Scanner*, const char*);

/**
 * Destroy the given scanner
 */
void scanner_destroy(Scanner);

/**
 * Perform a scan of the given source file
 * @param Scanner: the scanner that contains the source filename and the token list
 */ 
void scanner_scan_tokens(Scanner*);

/**
 * Print a report of the errors and warning in the scanner phase
 */
void scanner_errors_report(Scanner);

/**
 * Return true if an error has accoured in the scanner phase
 */
int scanner_had_error(Scanner);
#endif // !SCANNER_H
