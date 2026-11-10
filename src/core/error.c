#include "stem/stem.h"

const char* stem_error_message(stem_error_t error) {
    switch (error) {
        case STEM_OK:
            return "Success";
        case STEM_ERROR_MEMORY:
            return "Memory allocation failed";
        case STEM_ERROR_IO:
            return "I/O error";
        case STEM_ERROR_NOT_FOUND:
            return "Not found";
        case STEM_ERROR_EXISTS:
            return "Already exists";
        case STEM_ERROR_INVALID_ARG:
            return "Invalid argument";
        case STEM_ERROR_CRYPTO:
            return "Cryptography error";
        case STEM_ERROR_NETWORK:
            return "Network error";
        case STEM_ERROR_FORMAT:
            return "Format error";
        case STEM_ERROR_CONFLICT:
            return "Conflict detected";
        case STEM_ERROR_NOT_INITIALIZED:
            return "Not initialized";
        case STEM_ERROR_PERMISSION:
            return "Permission denied";
        default:
            return "Unknown error";
    }
}
