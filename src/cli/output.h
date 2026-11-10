#ifndef STEM_CLI_OUTPUT_H
#define STEM_CLI_OUTPUT_H

#include <stdio.h>

/* Terminal colors (ANSI escape codes) */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GRAY    "\033[90m"

/* Output functions */
void print_success(const char *fmt, ...);
void print_error(const char *fmt, ...);
void print_warning(const char *fmt, ...);
void print_info(const char *fmt, ...);

#endif /* STEM_CLI_OUTPUT_H */
