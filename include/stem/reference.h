#ifndef STEM_REFERENCE_H
#define STEM_REFERENCE_H

#include "stem/types.h"
#include "stem/repository.h"

/* Reference types */
typedef enum {
    STEM_REF_DIRECT,    /* Points directly to an object hash */
    STEM_REF_SYMBOLIC   /* Points to another reference */
} stem_ref_type_t;

/* Reference structure */
struct stem_reference {
    char *name;
    stem_ref_type_t type;
    union {
        stem_hash_t direct;
        char *symbolic;
    } target;
};

/* Reference operations */
STEM_API stem_error_t stem_reference_lookup(
    stem_repository_t *repo,
    const char *name,
    stem_reference_t **ref_out
);

STEM_API stem_error_t stem_reference_create(
    stem_repository_t *repo,
    const char *name,
    const stem_hash_t *target,
    bool force
);

STEM_API stem_error_t stem_reference_create_symbolic(
    stem_repository_t *repo,
    const char *name,
    const char *target,
    bool force
);

STEM_API stem_error_t stem_reference_delete(
    stem_repository_t *repo,
    const char *name
);

STEM_API stem_error_t stem_reference_list(
    stem_repository_t *repo,
    const char *pattern,
    char ***refs_out,
    size_t *count_out
);

STEM_API stem_error_t stem_reference_resolve(
    stem_repository_t *repo,
    const stem_reference_t *ref,
    stem_hash_t *hash_out
);

STEM_API void stem_reference_free(stem_reference_t *ref);

/* Branch operations */
STEM_API stem_error_t stem_branch_create(
    stem_repository_t *repo,
    const char *name,
    const stem_hash_t *commit
);

STEM_API stem_error_t stem_branch_delete(
    stem_repository_t *repo,
    const char *name
);

STEM_API stem_error_t stem_branch_list(
    stem_repository_t *repo,
    char ***branches_out,
    size_t *count_out
);

STEM_API stem_error_t stem_branch_current(
    stem_repository_t *repo,
    char **name_out
);

STEM_API stem_error_t stem_branch_checkout(
    stem_repository_t *repo,
    const char *name
);

#endif /* STEM_REFERENCE_H */
