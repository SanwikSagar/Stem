#ifndef STEM_INTERNAL_H
#define STEM_INTERNAL_H

#include "stem/types.h"
#include <stdio.h>

/* Internal utility functions */
void stem_set_last_error(const char *msg);

/* Buffer operations */
stem_error_t stem_buffer_init(stem_buffer_t *buf, size_t initial_capacity);
stem_error_t stem_buffer_append(stem_buffer_t *buf, const void *data, size_t len);
stem_error_t stem_buffer_append_str(stem_buffer_t *buf, const char *str);
void stem_buffer_free(stem_buffer_t *buf);

/* Path utilities */
char* stem_path_join(const char *base, const char *component);
char* stem_path_normalize(const char *path);
bool stem_path_is_absolute(const char *path);

/* File utilities */
stem_error_t stem_file_read(const char *path, uint8_t **data_out, size_t *size_out);
stem_error_t stem_file_write(const char *path, const uint8_t *data, size_t size);
bool stem_file_exists(const char *path);

#endif /* STEM_INTERNAL_H */
