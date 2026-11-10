#ifndef STEM_CRYPTO_H
#define STEM_CRYPTO_H

#include "stem/types.h"

/* Key sizes */
#define STEM_KEY_SIZE 32
#define STEM_NONCE_SIZE 12
#define STEM_TAG_SIZE 16

/* Encryption context */
typedef struct {
    uint8_t key[STEM_KEY_SIZE];
    bool initialized;
    void *_priv;
} stem_crypto_ctx_t;

/* Hash operations */
STEM_API stem_error_t stem_hash_data(
    const uint8_t *data,
    size_t size,
    stem_hash_t *hash_out
);

STEM_API stem_error_t stem_hash_file(
    const char *path,
    stem_hash_t *hash_out
);

STEM_API void stem_hash_to_hex(
    const stem_hash_t *hash,
    char *hex_out
);

STEM_API stem_error_t stem_hash_from_hex(
    const char *hex,
    stem_hash_t *hash_out
);

/* Encryption operations */
STEM_API stem_error_t stem_crypto_init(
    const uint8_t *key,
    size_t key_size,
    stem_crypto_ctx_t **ctx_out
);

STEM_API stem_error_t stem_crypto_encrypt(
    stem_crypto_ctx_t *ctx,
    const uint8_t *plaintext,
    size_t plaintext_size,
    uint8_t **ciphertext_out,
    size_t *ciphertext_size_out
);

STEM_API stem_error_t stem_crypto_decrypt(
    stem_crypto_ctx_t *ctx,
    const uint8_t *ciphertext,
    size_t ciphertext_size,
    uint8_t **plaintext_out,
    size_t *plaintext_size_out
);

STEM_API void stem_crypto_free(stem_crypto_ctx_t *ctx);

/* Key derivation */
STEM_API stem_error_t stem_kdf_derive(
    const char *password,
    const uint8_t *salt,
    size_t salt_size,
    uint32_t iterations,
    uint8_t *key_out,
    size_t key_size
);

/* Random generation */
STEM_API stem_error_t stem_random_bytes(
    uint8_t *buffer,
    size_t size
);

#endif /* STEM_CRYPTO_H */
