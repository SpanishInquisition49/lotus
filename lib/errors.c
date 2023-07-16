#include "errors.h"
#include <stdarg.h>
#include <stdio.h>

static char* pretty_log_level(int);
static char* get_ansi_color(int);
static enum log_level_t internal_log_level = INFO;

void err_log(enum log_level_t level, const char *restrict fmt, ...) {
    if(level > LOG_LEVELS || level < internal_log_level) return;
    va_list args;
    va_start(args, fmt);
    if(level == INFO) {
        dprintf(2,"%s[%s] " ANSI_COLOR_RESET, get_ansi_color(level), pretty_log_level(level));
        vdprintf(2, fmt, args);
    }
    else {
        dprintf(2,"%s[%s] ", get_ansi_color(level), pretty_log_level(level));
        vdprintf(2, fmt, args);
        dprintf(2, ANSI_COLOR_RESET);
    
    }
    va_end(args);
}

void err_log_v(enum log_level_t level, const char *restrict fmt, va_list args) {
    if(level > LOG_LEVELS || level < internal_log_level) return;
    if(level == INFO) {
        dprintf(2,"%s[%s] " ANSI_COLOR_RESET, get_ansi_color(level), pretty_log_level(level));
        vdprintf(2, fmt, args);
    }
    else {
        dprintf(2,"%s[%s] ", get_ansi_color(level), pretty_log_level(level));
        vdprintf(2, fmt, args);
        dprintf(2, ANSI_COLOR_RESET);
    }
}

void err_log_set_level(enum log_level_t level) {
    if(level > LOG_LEVELS) return;
    internal_log_level = level;
    return;
}

char *pretty_log_level(int level) {
    switch (level) {
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        default:
            __builtin_unreachable();
    }
}

char *get_ansi_color(int level) {
    switch (level) {
        case INFO:
            return ANSI_COLOR_BLUE;
        case WARNING:
            return ANSI_COLOR_YELLOW;
        case ERROR:
            return ANSI_COLOR_RED;
        default:
            return ANSI_COLOR_RESET;
    }
}
