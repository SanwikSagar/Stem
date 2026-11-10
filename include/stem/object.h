#ifndef STEM_OBJECT_H
#define STEM_OBJECT_H

#include "stem/types.h"
#include "stem/repository.h"

/* Object header */
typedef struct {
    stem_object_type_t type;
    size_t size;
    stem_hash_t hash;
} stem_object_header_t;

/* Blob object */
typedef struct {
    stem_object_header_t header;
    uint8_t *data;
    size_t size;
} stem_blob_t;

/* Tree entry */
typedef struct stem_tree_entry {
    char *name;
    uint32_t mode;
    stem_hash_t hash;
    stem_object_type_t type;
    struct stem_tree_entry *next;
} stem_tree_entry_t;

/* Tree object */
typedef struct {
    stem_object_header_t header;
    stem_tree_entry_t *entries;
    size_t entry_count;
} stem_tree_t;

/* Commit signature */
typedef struct {
    char *name;
    char *email;
    stem_time_t time;
    int32_t tz_offset;
} stem_signature_t;

/* Commit object */
typedef struct {
    stem_object_header_t header;
    stem_hash_t tree;
    stem_hash_t *parents;
    size_t parent_count;
    stem_signature_t author;
    stem_signature_t committer;
    char *message;
} stem_commit_t;

/* Object operations */
STEM_API stem_error_t stem_object_read(
    stem_repository_t *repo,
    const stem_hash_t *hash,
    stem_object_header_t **obj_out
);

STEM_API stem_error_t stem_object_write(
    stem_repository_t *repo,
    const stem_object_header_t *obj,
    stem_hash_t *hash_out
);

STEM_API void stem_object_free(stem_object_header_t *obj);

/* Blob operations */
STEM_API stem_error_t stem_blob_create(
    const uint8_t *data,
    size_t size,
    stem_blob_t **blob_out
);

STEM_API stem_error_t stem_blob_from_file(
    const char *path,
    stem_blob_t **blob_out
);

STEM_API void stem_blob_free(stem_blob_t *blob);

/* Tree operations */
STEM_API stem_error_t stem_tree_create(stem_tree_t **tree_out);

STEM_API stem_error_t stem_tree_add_entry(
    stem_tree_t *tree,
    const char *name,
    uint32_t mode,
    const stem_hash_t *hash,
    stem_object_type_t type
);

STEM_API stem_tree_entry_t* stem_tree_find_entry(
    const stem_tree_t *tree,
    const char *name
);

STEM_API void stem_tree_free(stem_tree_t *tree);

/* Commit operations */
STEM_API stem_error_t stem_commit_create(
    const stem_hash_t *tree,
    const stem_hash_t *parents,
    size_t parent_count,
    const stem_signature_t *author,
    const stem_signature_t *committer,
    const char *message,
    stem_commit_t **commit_out
);

STEM_API void stem_commit_free(stem_commit_t *commit);

/* Signature operations */
STEM_API stem_error_t stem_signature_create(
    const char *name,
    const char *email,
    stem_signature_t *sig_out
);

STEM_API void stem_signature_free(stem_signature_t *sig);

#endif /* STEM_OBJECT_H */
