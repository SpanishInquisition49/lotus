#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../lib/scanner.h"
#include "../lib/parser.h"
#include "../lib/interpreter.h"
#include "../lib/list.h"
#include "../lib/config.h"

static List run_scanner(const char*);
static List run_parser(List);
static void run_interpreter(List);
static void set_config(void);
static void* sig_handler(void*);
static void clean(void);

static int scanner_error = 0; 
static int parser_error = 0;
static int show_reports = 0;

static int scanner_alive = 0;
static int parser_alive = 0;
static int interpreter_alive = 0;

static Scanner scanner;
static Parser parser;
static Interpreter interpreter;

int main(int argc, char *argv[]) {
    // Installing sig_handler
    sigset_t sigs_all;
    sigfillset(&sigs_all);
    int s = pthread_sigmask(SIG_BLOCK, &sigs_all, NULL);
    if(s != 0) {
        dprintf(2, "pthread_sigmask: %s\n", strerror(s));
        exit(EXIT_FAILURE);
    }
    pthread_t sig_handler_thread;
    pthread_create(&sig_handler_thread, NULL, sig_handler, NULL);
    // Input validation
    set_config();
    if(argc != 2) {
        printf("Usage: main [filename]\n");
        return EXIT_FAILURE;
    }
    List tokens = run_scanner(argv[1]);
    if(scanner_error) {
        list_free(tokens, token_free);
        exit(EXIT_FAILURE);
    }
    List statements = run_parser(tokens);
    if(parser_error) {
        list_free(statements, stmt_free);
        exit(EXIT_FAILURE);
    }
    run_interpreter(statements);
    return EXIT_SUCCESS;
}

List run_scanner(const char* filename) {
    scanner_init(&scanner, filename);
    scanner_alive = 1;
    scanner_scan_tokens(&scanner);
    List tokens = NULL;
    tokens_dup(scanner.tokens, &tokens);
    scanner_error = scanner_had_error(scanner);
    if(show_reports || scanner_error)
        scanner_errors_report(scanner);
    scanner_destroy(scanner);
    scanner_alive = 0;
    return tokens;
}

List run_parser(List tokens) {
    parser_init(&parser, tokens);
    parser_alive = 1;
    List statements = parser_parse(&parser);
    parser_error = parser_had_errors(parser);
    if(show_reports || parser_error)
        parser_errors_report(parser);
    parser_destroy(parser);
    parser_alive = 0;
    return statements;
}

void run_interpreter(List statements) {
    interpreter_init(&interpreter, statements);
    interpreter_alive = 1;
    interpreter_eval(&interpreter);
    interpreter_destroy(interpreter);
    interpreter_alive = 0;
    return;
}

void set_config(void) {
   char *v = config_read("LOG_LEVEL");
   int log_level;
   if(v == NULL) {
    log_level = WARNING;
   }
   else if(strcmp(v, "INFO\n") == 0)
       log_level = INFO;
   else if(strcmp(v, "ERROR\n") == 0)
        log_level = ERROR;
   else
    log_level = WARNING;
   if(v)
    free(v);
   Log_set_level(log_level);
   v = config_read("PRINT_REPORT");
   if(v == NULL)
    show_reports = 0;
   else if(strcmp(v, "TRUE\n") == 0)
    show_reports = 1;
   else
    show_reports = 0;
   if(v)
    free(v);
   return;
}

void *sig_handler(void *args) {
    int s = pthread_detach(pthread_self());
    if(s != 0) {
        dprintf(2, "pthread_detach\n");
        exit(EXIT_FAILURE);
    }
    int sig;
    sigset_t sigs_wait;
    sigemptyset(&sigs_wait);
    sigaddset(&sigs_wait, SIGINT);
    sigset_t sigs_all;
    sigfillset(&sigs_all);
    s = pthread_sigmask(SIG_BLOCK, &sigs_all, NULL);
    if(s != 0) {
        dprintf(2, "pthread_sigmask: %s\n", strerror(s));
        exit(EXIT_FAILURE);
    }

    for(;;) {
       s = sigwait(&sigs_wait, &sig);
       if(s != 0) {
            dprintf(2, "sigwait: %s\n", strerror(s));
            exit(EXIT_FAILURE);
       }
       if(sig == SIGINT) {
        dprintf(2, "Received SIGINT\n");
        clean();
        exit(EXIT_SUCCESS);
       }
    }

    return NULL;
}

void clean() {
    if(scanner_alive)
        scanner_destroy(scanner);
    if(parser_alive)
        parser_destroy(parser);
    if(interpreter_alive)
        interpreter_destroy(interpreter);
    return;
}

