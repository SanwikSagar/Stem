#ifndef STEM_CONFIG_H
#define STEM_CONFIG_H

#include "stem/types.h"

/* Config structure */
struct stem_config {
    void *_priv;
};

/* Config operations */
STEM_API stem_error_t stem_config_open(
    const char *path,
    stem_config_t **config_out
);

STEM_API stem_error_t stem_config_get_string(
    const stem_config_t *config,
    const char *section,
    const char *key,
    char **value_out
);

STEM_API stem_error_t stem_config_set_string(
    stem_config_t *config,
    const char *section,
    const char *key,
    const char *value
);

STEM_API stem_error_t stem_config_get_int(
    const stem_config_t *config,
    const char *section,
    const char *key,
    int64_t *value_out
);

STEM_API stem_error_t stem_config_set_int(
    stem_config_t *config,
    const char *section,
    const char *key,
    int64_t value
);

STEM_API stem_error_t stem_config_get_bool(
    const stem_config_t *config,
    const char *section,
    const char *key,
    bool *value_out
);

STEM_API stem_error_t stem_config_set_bool(
    stem_config_t *config,
    const char *section,
    const char *key,
    bool value
);

STEM_API stem_error_t stem_config_delete(
    stem_config_t *config,
    const char *section,
    const char *key
);

STEM_API stem_error_t stem_config_save(stem_config_t *config);

STEM_API void stem_config_free(stem_config_t *config);

#endif /* STEM_CONFIG_H */
