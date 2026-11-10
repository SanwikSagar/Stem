#include "stem/object.h"
#include "stem/crypto.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

stem_error_t stem_blob_create(const uint8_t *data, size_t size, stem_blob_t **blob_out) {
    if (!data || !blob_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    stem_blob_t *blob = calloc(1, sizeof(stem_blob_t));
    if (!blob) return STEM_ERROR_MEMORY;
    
    blob->data = malloc(size);
    if (!blob->data) {
        free(blob);
        return STEM_ERROR_MEMORY;
    }
    
    memcpy(blob->data, data, size);
    blob->size = size;
    blob->header.type = STEM_OBJ_BLOB;
    blob->header.size = size;
    
    /* Compute hash */
    stem_hash_data(data, size, &blob->header.hash);
    
    *blob_out = blob;
    return STEM_OK;
}

stem_error_t stem_blob_from_file(const char *path, stem_blob_t **blob_out) {
    if (!path || !blob_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    uint8_t *data = NULL;
    size_t size = 0;
    
    stem_error_t err = stem_file_read(path, &data, &size);
    if (err != STEM_OK) return err;
    
    err = stem_blob_create(data, size, blob_out);
    free(data);
    
    return err;
}

void stem_blob_free(stem_blob_t *blob) {
    if (!blob) return;
    free(blob->data);
    free(blob);
}

stem_error_t stem_tree_create(stem_tree_t **tree_out) {
    if (!tree_out) return STEM_ERROR_INVALID_ARG;
    
    stem_tree_t *tree = calloc(1, sizeof(stem_tree_t));
    if (!tree) return STEM_ERROR_MEMORY;
    
    tree->header.type = STEM_OBJ_TREE;
    
    *tree_out = tree;
    return STEM_OK;
}

stem_error_t stem_tree_add_entry(stem_tree_t *tree, const char *name,
                                  uint32_t mode, const stem_hash_t *hash,
                                  stem_object_type_t type) {
    if (!tree || !name || !hash) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    stem_tree_entry_t *entry = calloc(1, sizeof(stem_tree_entry_t));
    if (!entry) return STEM_ERROR_MEMORY;
    
    entry->name = strdup(name);
    if (!entry->name) {
        free(entry);
        return STEM_ERROR_MEMORY;
    }
    
    entry->mode = mode;
    entry->hash = *hash;
    entry->type = type;
    entry->next = tree->entries;
    
    tree->entries = entry;
    tree->entry_count++;
    
    return STEM_OK;
}

stem_tree_entry_t* stem_tree_find_entry(const stem_tree_t *tree, const char *name) {
    if (!tree || !name) return NULL;
    
    for (stem_tree_entry_t *e = tree->entries; e; e = e->next) {
        if (strcmp(e->name, name) == 0) {
            return e;
        }
    }
    
    return NULL;
}

void stem_tree_free(stem_tree_t *tree) {
    if (!tree) return;
    
    stem_tree_entry_t *entry = tree->entries;
    while (entry) {
        stem_tree_entry_t *next = entry->next;
        free(entry->name);
        free(entry);
        entry = next;
    }
    
    free(tree);
}

stem_error_t stem_commit_create(const stem_hash_t *tree,
                                 const stem_hash_t *parents,
                                 size_t parent_count,
                                 const stem_signature_t *author,
                                 const stem_signature_t *committer,
                                 const char *message,
                                 stem_commit_t **commit_out) {
    if (!tree || !author || !committer || !message || !commit_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    stem_commit_t *commit = calloc(1, sizeof(stem_commit_t));
    if (!commit) return STEM_ERROR_MEMORY;
    
    commit->header.type = STEM_OBJ_COMMIT;
    commit->tree = *tree;
    commit->parent_count = parent_count;
    
    if (parent_count > 0) {
        commit->parents = malloc(parent_count * sizeof(stem_hash_t));
        if (!commit->parents) {
            free(commit);
            return STEM_ERROR_MEMORY;
        }
        memcpy(commit->parents, parents, parent_count * sizeof(stem_hash_t));
    }
    
    commit->author = *author;
    commit->committer = *committer;
    commit->message = strdup(message);
    
    if (!commit->message) {
        free(commit->parents);
        free(commit);
        return STEM_ERROR_MEMORY;
    }
    
    *commit_out = commit;
    return STEM_OK;
}

void stem_commit_free(stem_commit_t *commit) {
    if (!commit) return;
    
    free(commit->parents);
    free(commit->message);
    stem_signature_free(&commit->author);
    stem_signature_free(&commit->committer);
    free(commit);
}

stem_error_t stem_signature_create(const char *name, const char *email,
                                    stem_signature_t *sig_out) {
    if (!name || !email || !sig_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    sig_out->name = strdup(name);
    sig_out->email = strdup(email);
    
    if (!sig_out->name || !sig_out->email) {
        free(sig_out->name);
        free(sig_out->email);
        return STEM_ERROR_MEMORY;
    }
    
    time_t now = time(NULL);
    sig_out->time.seconds = now;
    sig_out->time.nanoseconds = 0;
    sig_out->tz_offset = 0;
    
    return STEM_OK;
}

void stem_signature_free(stem_signature_t *sig) {
    if (!sig) return;
    free(sig->name);
    free(sig->email);
}
