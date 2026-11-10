#include "stem/crypto.h"
#include "internal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef STEM_ENABLE_ENCRYPTION
#include <openssl/evp.h>

stem_error_t stem_hash_data(const uint8_t *data, size_t size, stem_hash_t *hash_out) {
    if (!data || !hash_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        return STEM_ERROR_CRYPTO;
    }
    
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(ctx, data, size) != 1 ||
        EVP_DigestFinal_ex(ctx, hash_out->bytes, NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        return STEM_ERROR_CRYPTO;
    }
    
    EVP_MD_CTX_free(ctx);
    return STEM_OK;
}

#else

/* Simple hash implementation without OpenSSL */
static uint32_t hash32(const uint8_t *data, size_t len) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

stem_error_t stem_hash_data(const uint8_t *data, size_t size, stem_hash_t *hash_out) {
    if (!data || !hash_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    /* Simple hash for testing without OpenSSL */
    memset(hash_out->bytes, 0, STEM_HASH_SIZE);
    uint32_t h = hash32(data, size);
    memcpy(hash_out->bytes, &h, sizeof(h));
    
    return STEM_OK;
}

#endif

stem_error_t stem_hash_file(const char *path, stem_hash_t *hash_out) {
    if (!path || !hash_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    uint8_t *data = NULL;
    size_t size = 0;
    
    stem_error_t err = stem_file_read(path, &data, &size);
    if (err != STEM_OK) {
        return err;
    }
    
    err = stem_hash_data(data, size, hash_out);
    free(data);
    
    return err;
}

void stem_hash_to_hex(const stem_hash_t *hash, char *hex_out) {
    if (!hash || !hex_out) return;
    
    for (int i = 0; i < STEM_HASH_SIZE; i++) {
        sprintf(hex_out + (i * 2), "%02x", hash->bytes[i]);
    }
    hex_out[STEM_HASH_HEX_SIZE - 1] = '\0';
}

stem_error_t stem_hash_from_hex(const char *hex, stem_hash_t *hash_out) {
    if (!hex || !hash_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    if (strlen(hex) != STEM_HASH_HEX_SIZE - 1) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    for (int i = 0; i < STEM_HASH_SIZE; i++) {
        unsigned int byte;
        if (sscanf(hex + (i * 2), "%2x", &byte) != 1) {
            return STEM_ERROR_FORMAT;
        }
        hash_out->bytes[i] = (uint8_t)byte;
    }
    
    return STEM_OK;
}
