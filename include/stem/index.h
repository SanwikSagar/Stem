#ifndef STEM_INDEX_H
#define STEM_INDEX_H

#include "stem/types.h"
#include "stem/repository.h"

/* Index entry */
typedef struct {
    char *path;
    stem_hash_t hash;
    uint32_t mode;
    uint64_t size;
    stem_time_t mtime;
    uint32_t flags;
} stem_index_entry_t;

/* Index structure */
struct stem_index {
    stem_index_entry_t *entries;
    size_t entry_count;
    size_t entry_capacity;
    bool modified;
    void *_priv;
};

/* Index operations */
STEM_API stem_error_t stem_index_open(
    stem_repository_t *repo,
    stem_index_t **index_out
);

STEM_API stem_error_t stem_index_read(stem_index_t *index);

STEM_API stem_error_t stem_index_write(stem_index_t *index);

STEM_API stem_error_t stem_index_add(
    stem_index_t *index,
    const char *path,
    const stem_hash_t *hash,
    uint32_t mode,
    uint64_t size
);

STEM_API stem_error_t stem_index_remove(
    stem_index_t *index,
    const char *path
);

STEM_API stem_index_entry_t* stem_index_find(
    const stem_index_t *index,
    const char *path
);

STEM_API void stem_index_clear(stem_index_t *index);

STEM_API void stem_index_free(stem_index_t *index);

#endif /* STEM_INDEX_H */
