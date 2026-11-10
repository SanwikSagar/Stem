#ifndef STEM_MERGE_H
#define STEM_MERGE_H

#include "stem/types.h"
#include "stem/repository.h"

/* Merge strategy */
typedef enum {
    STEM_MERGE_STRATEGY_RECURSIVE,
    STEM_MERGE_STRATEGY_OURS,
    STEM_MERGE_STRATEGY_THEIRS,
    STEM_MERGE_STRATEGY_OCTOPUS
} stem_merge_strategy_t;

/* Merge conflict */
typedef struct {
    char *path;
    stem_hash_t ours;
    stem_hash_t theirs;
    stem_hash_t base;
} stem_merge_conflict_t;

/* Merge result */
typedef struct {
    bool has_conflicts;
    stem_merge_conflict_t *conflicts;
    size_t conflict_count;
    stem_hash_t result_tree;
} stem_merge_result_t;

/* Merge operations */
STEM_API stem_error_t stem_merge(
    stem_repository_t *repo,
    const stem_hash_t *ours,
    const stem_hash_t *theirs,
    stem_merge_strategy_t strategy,
    stem_merge_result_t **result_out
);

STEM_API stem_error_t stem_merge_commits(
    stem_repository_t *repo,
    const char *their_branch,
    stem_merge_strategy_t strategy,
    stem_merge_result_t **result_out
);

STEM_API void stem_merge_result_free(stem_merge_result_t *result);

#endif /* STEM_MERGE_H */
