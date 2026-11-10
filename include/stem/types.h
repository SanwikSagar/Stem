#ifndef STEM_TYPES_H
#define STEM_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Platform-specific definitions */
#ifdef _WIN32
    #ifdef STEM_BUILD_SHARED
        #define STEM_API __declspec(dllexport)
    #elif defined(STEM_USE_SHARED)
        #define STEM_API __declspec(dllimport)
    #else
        #define STEM_API
    #endif
    #define STEM_PATH_SEP '\\'
    #define STEM_PATH_SEP_STR "\\"
#else
    #define STEM_API __attribute__((visibility("default")))
    #define STEM_PATH_SEP '/'
    #define STEM_PATH_SEP_STR "/"
#endif

/* Hash types */
#define STEM_HASH_SIZE 32
#define STEM_HASH_HEX_SIZE (STEM_HASH_SIZE * 2 + 1)

typedef struct {
    uint8_t bytes[STEM_HASH_SIZE];
} stem_hash_t;

/* Error codes */
typedef enum {
    STEM_OK = 0,
    STEM_ERROR_MEMORY = -1,
    STEM_ERROR_IO = -2,
    STEM_ERROR_NOT_FOUND = -3,
    STEM_ERROR_EXISTS = -4,
    STEM_ERROR_INVALID_ARG = -5,
    STEM_ERROR_CRYPTO = -6,
    STEM_ERROR_NETWORK = -7,
    STEM_ERROR_FORMAT = -8,
    STEM_ERROR_CONFLICT = -9,
    STEM_ERROR_NOT_INITIALIZED = -10,
    STEM_ERROR_PERMISSION = -11
} stem_error_t;

/* Object types */
typedef enum {
    STEM_OBJ_BLOB = 1,
    STEM_OBJ_TREE = 2,
    STEM_OBJ_COMMIT = 3,
    STEM_OBJ_TAG = 4
} stem_object_type_t;

/* Time structure */
typedef struct {
    int64_t seconds;
    int32_t nanoseconds;
} stem_time_t;

/* String buffer */
typedef struct {
    char *data;
    size_t len;
    size_t capacity;
} stem_buffer_t;

/* Forward declarations */
typedef struct stem_repository stem_repository_t;
typedef struct stem_config stem_config_t;
typedef struct stem_index stem_index_t;
typedef struct stem_reference stem_reference_t;
typedef struct stem_object_header stem_object_t;

/* Common result type */
#define STEM_RESULT(T) \
    struct { \
        T value; \
        stem_error_t error; \
        char *error_msg; \
    }

#endif /* STEM_TYPES_H */
