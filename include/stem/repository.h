#ifndef STEM_REPOSITORY_H
#define STEM_REPOSITORY_H

#include "stem/types.h"

/* Repository structure */
struct stem_repository {
    char *path;
    char *stem_dir;
    stem_config_t *config;
    stem_index_t *index;
    bool is_bare;
    void *_priv;
};

/* Repository operations */
STEM_API stem_error_t stem_repository_init(
    const char *path,
    bool bare,
    stem_repository_t **repo_out
);

STEM_API stem_error_t stem_repository_open(
    const char *path,
    stem_repository_t **repo_out
);

STEM_API void stem_repository_free(stem_repository_t *repo);

STEM_API stem_error_t stem_repository_discover(
    const char *start_path,
    char **repo_path_out
);

STEM_API bool stem_repository_is_valid(const stem_repository_t *repo);

STEM_API const char* stem_repository_path(const stem_repository_t *repo);

STEM_API const char* stem_repository_stem_dir(const stem_repository_t *repo);

#endif /* STEM_REPOSITORY_H */
