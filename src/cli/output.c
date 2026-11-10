#include "output.h"
#include <stdarg.h>

void print_success(const char *fmt, ...) {
    printf(COLOR_GREEN "[OK]" COLOR_RESET " ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void print_error(const char *fmt, ...) {
    fprintf(stderr, COLOR_RED "[ERROR]" COLOR_RESET " ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void print_warning(const char *fmt, ...) {
    printf(COLOR_YELLOW "[WARNING]" COLOR_RESET " ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void print_info(const char *fmt, ...) {
    printf(COLOR_BLUE "[*]" COLOR_RESET " ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}
