#include "stem/reference.h"
#include "stem/crypto.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char* ref_path(stem_repository_t *repo, const char *name) {
    return stem_path_join(stem_repository_stem_dir(repo), name);
}

stem_error_t stem_reference_lookup(stem_repository_t *repo, const char *name,
                                    stem_reference_t **ref_out) {
    if (!repo || !name || !ref_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    char *path = ref_path(repo, name);
    if (!path) return STEM_ERROR_MEMORY;
    
    if (!stem_file_exists(path)) {
        free(path);
        return STEM_ERROR_NOT_FOUND;
    }
    
    uint8_t *data = NULL;
    size_t size = 0;
    stem_error_t err = stem_file_read(path, &data, &size);
    free(path);
    
    if (err != STEM_OK) return err;
    
    stem_reference_t *ref = calloc(1, sizeof(stem_reference_t));
    if (!ref) {
        free(data);
        return STEM_ERROR_MEMORY;
    }
    
    ref->name = strdup(name);
    if (!ref->name) {
        free(ref);
        free(data);
        return STEM_ERROR_MEMORY;
    }
    
    /* Parse reference */
    char *line = (char*)data;
    if (strncmp(line, "ref:", 4) == 0) {
        /* Symbolic reference */
        ref->type = STEM_REF_SYMBOLIC;
        char *target = line + 4;
        while (*target == ' ' || *target == '\t') target++;
        
        /* Remove trailing newline */
        char *end = strchr(target, '\n');
        if (end) *end = '\0';
        
        ref->target.symbolic = strdup(target);
        if (!ref->target.symbolic) {
            free(ref->name);
            free(ref);
            free(data);
            return STEM_ERROR_MEMORY;
        }
    } else {
        /* Direct reference */
        ref->type = STEM_REF_DIRECT;
        char hex[STEM_HASH_HEX_SIZE];
        strncpy(hex, line, STEM_HASH_HEX_SIZE - 1);
        hex[STEM_HASH_HEX_SIZE - 1] = '\0';
        
        err = stem_hash_from_hex(hex, &ref->target.direct);
        if (err != STEM_OK) {
            free(ref->name);
            free(ref);
            free(data);
            return err;
        }
    }
    
    free(data);
    *ref_out = ref;
    return STEM_OK;
}

stem_error_t stem_reference_create(stem_repository_t *repo, const char *name,
                                    const stem_hash_t *target, bool force) {
    if (!repo || !name || !target) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    char *path = ref_path(repo, name);
    if (!path) return STEM_ERROR_MEMORY;
    
    if (!force && stem_file_exists(path)) {
        free(path);
        return STEM_ERROR_EXISTS;
    }
    
    char hex[STEM_HASH_HEX_SIZE];
    stem_hash_to_hex(target, hex);
    
    stem_error_t err = stem_file_write(path, (uint8_t*)hex, strlen(hex));
    free(path);
    
    return err;
}

stem_error_t stem_reference_create_symbolic(stem_repository_t *repo, const char *name,
                                             const char *target, bool force) {
    if (!repo || !name || !target) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    char *path = ref_path(repo, name);
    if (!path) return STEM_ERROR_MEMORY;
    
    if (!force && stem_file_exists(path)) {
        free(path);
        return STEM_ERROR_EXISTS;
    }
    
    char buf[512];
    snprintf(buf, sizeof(buf), "ref: %s\n", target);
    
    stem_error_t err = stem_file_write(path, (uint8_t*)buf, strlen(buf));
    free(path);
    
    return err;
}

void stem_reference_free(stem_reference_t *ref) {
    if (!ref) return;
    
    free(ref->name);
    if (ref->type == STEM_REF_SYMBOLIC) {
        free(ref->target.symbolic);
    }
    free(ref);
}

stem_error_t stem_branch_create(stem_repository_t *repo, const char *name,
                                 const stem_hash_t *commit) {
    if (!repo || !name || !commit) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    char ref_name[256];
    snprintf(ref_name, sizeof(ref_name), "refs/heads/%s", name);
    
    return stem_reference_create(repo, ref_name, commit, false);
}

stem_error_t stem_branch_current(stem_repository_t *repo, char **name_out) {
    if (!repo || !name_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    stem_reference_t *head = NULL;
    stem_error_t err = stem_reference_lookup(repo, "HEAD", &head);
    if (err != STEM_OK) return err;
    
    if (head->type == STEM_REF_SYMBOLIC) {
        /* Extract branch name from refs/heads/... */
        const char *ref = head->target.symbolic;
        const char *prefix = "refs/heads/";
        if (strncmp(ref, prefix, strlen(prefix)) == 0) {
            *name_out = strdup(ref + strlen(prefix));
            stem_reference_free(head);
            return *name_out ? STEM_OK : STEM_ERROR_MEMORY;
        }
    }
    
    stem_reference_free(head);
    return STEM_ERROR_NOT_FOUND;
}
