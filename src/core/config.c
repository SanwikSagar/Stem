#include "stem/config.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>

/* Simple INI-style config implementation */
typedef struct config_entry {
    char *section;
    char *key;
    char *value;
    struct config_entry *next;
} config_entry_t;

typedef struct {
    char *path;
    config_entry_t *entries;
    bool modified;
} config_impl_t;

stem_error_t stem_config_open(const char *path, stem_config_t **config_out) {
    if (!path || !config_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    stem_config_t *config = calloc(1, sizeof(stem_config_t));
    if (!config) {
        return STEM_ERROR_MEMORY;
    }
    
    config_impl_t *impl = calloc(1, sizeof(config_impl_t));
    if (!impl) {
        free(config);
        return STEM_ERROR_MEMORY;
    }
    
    impl->path = strdup(path);
    if (!impl->path) {
        free(impl);
        free(config);
        return STEM_ERROR_MEMORY;
    }
    
    config->_priv = impl;
    
    /* Try to load existing config */
    if (stem_file_exists(path)) {
        /* TODO: Parse config file */
    }
    
    *config_out = config;
    return STEM_OK;
}

static config_entry_t* find_entry(config_impl_t *impl, const char *section, const char *key) {
    for (config_entry_t *e = impl->entries; e; e = e->next) {
        if (strcmp(e->section, section) == 0 && strcmp(e->key, key) == 0) {
            return e;
        }
    }
    return NULL;
}

static stem_error_t set_entry(config_impl_t *impl, const char *section, const char *key, const char *value) {
    config_entry_t *entry = find_entry(impl, section, key);
    
    if (entry) {
        free(entry->value);
        entry->value = strdup(value);
        if (!entry->value) return STEM_ERROR_MEMORY;
    } else {
        entry = calloc(1, sizeof(config_entry_t));
        if (!entry) return STEM_ERROR_MEMORY;
        
        entry->section = strdup(section);
        entry->key = strdup(key);
        entry->value = strdup(value);
        
        if (!entry->section || !entry->key || !entry->value) {
            free(entry->section);
            free(entry->key);
            free(entry->value);
            free(entry);
            return STEM_ERROR_MEMORY;
        }
        
        entry->next = impl->entries;
        impl->entries = entry;
    }
    
    impl->modified = true;
    return STEM_OK;
}

stem_error_t stem_config_get_string(const stem_config_t *config, const char *section, const char *key, char **value_out) {
    if (!config || !section || !key || !value_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    config_impl_t *impl = (config_impl_t*)config->_priv;
    config_entry_t *entry = find_entry(impl, section, key);
    
    if (!entry) {
        return STEM_ERROR_NOT_FOUND;
    }
    
    *value_out = strdup(entry->value);
    return *value_out ? STEM_OK : STEM_ERROR_MEMORY;
}

stem_error_t stem_config_set_string(stem_config_t *config, const char *section, const char *key, const char *value) {
    if (!config || !section || !key || !value) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    config_impl_t *impl = (config_impl_t*)config->_priv;
    return set_entry(impl, section, key, value);
}

stem_error_t stem_config_get_int(const stem_config_t *config, const char *section, const char *key, int64_t *value_out) {
    char *str = NULL;
    stem_error_t err = stem_config_get_string(config, section, key, &str);
    if (err != STEM_OK) return err;
    
    *value_out = atoll(str);
    free(str);
    return STEM_OK;
}

stem_error_t stem_config_set_int(stem_config_t *config, const char *section, const char *key, int64_t value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld", (long long)value);
    return stem_config_set_string(config, section, key, buf);
}

stem_error_t stem_config_get_bool(const stem_config_t *config, const char *section, const char *key, bool *value_out) {
    char *str = NULL;
    stem_error_t err = stem_config_get_string(config, section, key, &str);
    if (err != STEM_OK) return err;
    
    *value_out = (strcmp(str, "true") == 0 || strcmp(str, "1") == 0);
    free(str);
    return STEM_OK;
}

stem_error_t stem_config_set_bool(stem_config_t *config, const char *section, const char *key, bool value) {
    return stem_config_set_string(config, section, key, value ? "true" : "false");
}

stem_error_t stem_config_save(stem_config_t *config) {
    if (!config) return STEM_ERROR_INVALID_ARG;
    
    config_impl_t *impl = (config_impl_t*)config->_priv;
    if (!impl->modified) return STEM_OK;
    
    /* TODO: Write config file */
    impl->modified = false;
    return STEM_OK;
}

void stem_config_free(stem_config_t *config) {
    if (!config) return;
    
    config_impl_t *impl = (config_impl_t*)config->_priv;
    if (impl) {
        config_entry_t *entry = impl->entries;
        while (entry) {
            config_entry_t *next = entry->next;
            free(entry->section);
            free(entry->key);
            free(entry->value);
            free(entry);
            entry = next;
        }
        free(impl->path);
        free(impl);
    }
    
    free(config);
}
