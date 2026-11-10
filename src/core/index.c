#include "stem/index.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>

stem_error_t stem_index_open(stem_repository_t *repo, stem_index_t **index_out) {
    if (!repo || !index_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    stem_index_t *index = calloc(1, sizeof(stem_index_t));
    if (!index) {
        return STEM_ERROR_MEMORY;
    }
    
    index->entry_capacity = 16;
    index->entries = calloc(index->entry_capacity, sizeof(stem_index_entry_t));
    if (!index->entries) {
        free(index);
        return STEM_ERROR_MEMORY;
    }
    
    *index_out = index;
    return STEM_OK;
}

stem_error_t stem_index_read(stem_index_t *index) {
    if (!index) return STEM_ERROR_INVALID_ARG;
    /* TODO: Read index from file */
    return STEM_OK;
}

stem_error_t stem_index_write(stem_index_t *index) {
    if (!index) return STEM_ERROR_INVALID_ARG;
    /* TODO: Write index to file */
    index->modified = false;
    return STEM_OK;
}

stem_error_t stem_index_add(stem_index_t *index, const char *path, 
                             const stem_hash_t *hash, uint32_t mode, uint64_t size) {
    if (!index || !path || !hash) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    /* Check if entry exists */
    stem_index_entry_t *existing = stem_index_find(index, path);
    if (existing) {
        existing->hash = *hash;
        existing->mode = mode;
        existing->size = size;
        index->modified = true;
        return STEM_OK;
    }
    
    /* Grow array if needed */
    if (index->entry_count >= index->entry_capacity) {
        size_t new_cap = index->entry_capacity * 2;
        stem_index_entry_t *new_entries = realloc(index->entries, 
                                                   new_cap * sizeof(stem_index_entry_t));
        if (!new_entries) return STEM_ERROR_MEMORY;
        
        index->entries = new_entries;
        index->entry_capacity = new_cap;
    }
    
    /* Add new entry */
    stem_index_entry_t *entry = &index->entries[index->entry_count];
    entry->path = strdup(path);
    if (!entry->path) return STEM_ERROR_MEMORY;
    
    entry->hash = *hash;
    entry->mode = mode;
    entry->size = size;
    entry->flags = 0;
    
    index->entry_count++;
    index->modified = true;
    
    return STEM_OK;
}

stem_error_t stem_index_remove(stem_index_t *index, const char *path) {
    if (!index || !path) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    for (size_t i = 0; i < index->entry_count; i++) {
        if (strcmp(index->entries[i].path, path) == 0) {
            free(index->entries[i].path);
            
            /* Shift remaining entries */
            memmove(&index->entries[i], &index->entries[i + 1],
                    (index->entry_count - i - 1) * sizeof(stem_index_entry_t));
            
            index->entry_count--;
            index->modified = true;
            return STEM_OK;
        }
    }
    
    return STEM_ERROR_NOT_FOUND;
}

stem_index_entry_t* stem_index_find(const stem_index_t *index, const char *path) {
    if (!index || !path) return NULL;
    
    for (size_t i = 0; i < index->entry_count; i++) {
        if (strcmp(index->entries[i].path, path) == 0) {
            return &index->entries[i];
        }
    }
    
    return NULL;
}

void stem_index_clear(stem_index_t *index) {
    if (!index) return;
    
    for (size_t i = 0; i < index->entry_count; i++) {
        free(index->entries[i].path);
    }
    
    index->entry_count = 0;
    index->modified = true;
}

void stem_index_free(stem_index_t *index) {
    if (!index) return;
    
    stem_index_clear(index);
    free(index->entries);
    free(index);
}
