#ifndef STEM_STORAGE_H
#define STEM_STORAGE_H

#include "stem/types.h"

/* Storage provider types */
typedef enum {
    STEM_STORAGE_LOCAL,
    STEM_STORAGE_S3,
    STEM_STORAGE_WEBDAV,
    STEM_STORAGE_SSH,
    STEM_STORAGE_GDRIVE,
    STEM_STORAGE_DROPBOX,
    STEM_STORAGE_ONEDRIVE,
    STEM_STORAGE_CUSTOM
} stem_storage_type_t;

/* Storage provider interface */
typedef struct stem_storage_provider {
    stem_storage_type_t type;
    char *name;
    
    /* Function pointers */
    stem_error_t (*connect)(struct stem_storage_provider *provider);
    stem_error_t (*disconnect)(struct stem_storage_provider *provider);
    stem_error_t (*read)(struct stem_storage_provider *provider, const char *key, uint8_t **data_out, size_t *size_out);
    stem_error_t (*write)(struct stem_storage_provider *provider, const char *key, const uint8_t *data, size_t size);
    stem_error_t (*delete)(struct stem_storage_provider *provider, const char *key);
    stem_error_t (*exists)(struct stem_storage_provider *provider, const char *key, bool *exists_out);
    stem_error_t (*list)(struct stem_storage_provider *provider, const char *prefix, char ***keys_out, size_t *count_out);
    
    void *_priv;
} stem_storage_provider_t;

/* Storage manager */
typedef struct {
    stem_storage_provider_t **providers;
    size_t provider_count;
    stem_storage_provider_t *primary;
    void *_priv;
} stem_storage_manager_t;

/* Storage provider operations */
STEM_API stem_error_t stem_storage_provider_create_local(
    const char *path,
    stem_storage_provider_t **provider_out
);

STEM_API stem_error_t stem_storage_provider_create_s3(
    const char *bucket,
    const char *region,
    const char *access_key,
    const char *secret_key,
    const char *endpoint,
    stem_storage_provider_t **provider_out
);

STEM_API stem_error_t stem_storage_provider_create_webdav(
    const char *url,
    const char *username,
    const char *password,
    stem_storage_provider_t **provider_out
);

STEM_API stem_error_t stem_storage_provider_create_ssh(
    const char *host,
    int port,
    const char *username,
    const char *password,
    const char *key_path,
    const char *remote_path,
    stem_storage_provider_t **provider_out
);

STEM_API void stem_storage_provider_free(stem_storage_provider_t *provider);

/* Storage manager operations */
STEM_API stem_error_t stem_storage_manager_create(
    stem_storage_manager_t **manager_out
);

STEM_API stem_error_t stem_storage_manager_add_provider(
    stem_storage_manager_t *manager,
    stem_storage_provider_t *provider,
    bool set_as_primary
);

STEM_API stem_error_t stem_storage_manager_remove_provider(
    stem_storage_manager_t *manager,
    const char *name
);

STEM_API stem_error_t stem_storage_manager_sync(
    stem_storage_manager_t *manager,
    const char *key
);

STEM_API void stem_storage_manager_free(stem_storage_manager_t *manager);

#endif /* STEM_STORAGE_H */
