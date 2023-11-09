#ifndef ERRORS_H
#define ERRORS_H
#include <stdarg.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define LOG_LEVELS 3

enum log_level_t {
  INFO,
  WARNING,
  ERROR,
};

__attribute__((format(printf, 2, 3))) void err_log(enum log_level_t,
                                                   const char *restrict, ...);
void err_log_v(enum log_level_t, const char *restrict, va_list);

void err_log_set_level(enum log_level_t);

// void Log_report()

#endif // !ERRORS_H
