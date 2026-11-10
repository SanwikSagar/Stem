#include "internal.h"
#include <stdlib.h>
#include <string.h>

stem_error_t stem_buffer_init(stem_buffer_t *buf, size_t initial_capacity) {
    if (!buf) return STEM_ERROR_INVALID_ARG;
    
    buf->data = malloc(initial_capacity);
    if (!buf->data) return STEM_ERROR_MEMORY;
    
    buf->len = 0;
    buf->capacity = initial_capacity;
    return STEM_OK;
}

stem_error_t stem_buffer_append(stem_buffer_t *buf, const void *data, size_t len) {
    if (!buf || !data) return STEM_ERROR_INVALID_ARG;
    
    if (buf->len + len > buf->capacity) {
        size_t new_capacity = (buf->len + len) * 2;
        char *new_data = realloc(buf->data, new_capacity);
        if (!new_data) return STEM_ERROR_MEMORY;
        
        buf->data = new_data;
        buf->capacity = new_capacity;
    }
    
    memcpy(buf->data + buf->len, data, len);
    buf->len += len;
    return STEM_OK;
}

stem_error_t stem_buffer_append_str(stem_buffer_t *buf, const char *str) {
    return stem_buffer_append(buf, str, strlen(str));
}

void stem_buffer_free(stem_buffer_t *buf) {
    if (buf) {
        free(buf->data);
        buf->data = NULL;
        buf->len = 0;
        buf->capacity = 0;
    }
}

char* stem_path_join(const char *base, const char *component) {
    if (!base || !component) return NULL;
    
    size_t base_len = strlen(base);
    size_t comp_len = strlen(component);
    size_t total = base_len + comp_len + 2;
    
    char *result = malloc(total);
    if (!result) return NULL;
    
    strcpy(result, base);
    if (base_len > 0 && base[base_len - 1] != STEM_PATH_SEP) {
        result[base_len] = STEM_PATH_SEP;
        result[base_len + 1] = '\0';
    }
    strcat(result, component);
    
    return result;
}

char* stem_path_normalize(const char *path) {
    if (!path) return NULL;
    return strdup(path);
}

bool stem_path_is_absolute(const char *path) {
    if (!path) return false;
    
#ifdef _WIN32
    return (path[0] != '\0' && path[1] == ':') || 
           (path[0] == '\\' && path[1] == '\\');
#else
    return path[0] == '/';
#endif
}

stem_error_t stem_file_read(const char *path, uint8_t **data_out, size_t *size_out) {
    if (!path || !data_out || !size_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    FILE *f = fopen(path, "rb");
    if (!f) return STEM_ERROR_IO;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (size < 0) {
        fclose(f);
        return STEM_ERROR_IO;
    }
    
    uint8_t *data = malloc(size);
    if (!data) {
        fclose(f);
        return STEM_ERROR_MEMORY;
    }
    
    if (fread(data, 1, size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return STEM_ERROR_IO;
    }
    
    fclose(f);
    *data_out = data;
    *size_out = size;
    return STEM_OK;
}

stem_error_t stem_file_write(const char *path, const uint8_t *data, size_t size) {
    if (!path || !data) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    FILE *f = fopen(path, "wb");
    if (!f) return STEM_ERROR_IO;
    
    if (fwrite(data, 1, size, f) != size) {
        fclose(f);
        return STEM_ERROR_IO;
    }
    
    fclose(f);
    return STEM_OK;
}

bool stem_file_exists(const char *path) {
    if (!path) return false;
    FILE *f = fopen(path, "r");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}
