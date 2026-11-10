#include "stem/repository.h"
#include "stem/config.h"
#include "stem/index.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#define stat _stat
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#else
#include <unistd.h>
#endif

static bool path_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static bool is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

static stem_error_t create_directory(const char *path) {
    if (mkdir(path, 0755) != 0 && !path_exists(path)) {
        return STEM_ERROR_IO;
    }
    return STEM_OK;
}

static stem_error_t create_repo_structure(const char *stem_dir) {
    char path[1024];
    stem_error_t err;
    
    /* Create main .stem directory */
    err = create_directory(stem_dir);
    if (err != STEM_OK) return err;
    
    /* Create subdirectories */
    const char *subdirs[] = {
        "objects",
        "refs",
        "refs/heads",
        "refs/tags",
        "refs/remotes",
        NULL
    };
    
    for (int i = 0; subdirs[i]; i++) {
        snprintf(path, sizeof(path), "%s%s%s", stem_dir, STEM_PATH_SEP_STR, subdirs[i]);
        err = create_directory(path);
        if (err != STEM_OK) return err;
    }
    
    /* Create HEAD file */
    snprintf(path, sizeof(path), "%s%sHEAD", stem_dir, STEM_PATH_SEP_STR);
    FILE *f = fopen(path, "w");
    if (!f) return STEM_ERROR_IO;
    fprintf(f, "ref: refs/heads/main\n");
    fclose(f);
    
    return STEM_OK;
}

stem_error_t stem_repository_init(const char *path, bool bare, stem_repository_t **repo_out) {
    if (!path || !repo_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    stem_repository_t *repo = calloc(1, sizeof(stem_repository_t));
    if (!repo) {
        return STEM_ERROR_MEMORY;
    }
    
    repo->path = strdup(path);
    if (!repo->path) {
        free(repo);
        return STEM_ERROR_MEMORY;
    }
    
    /* Create .stem directory path */
    size_t stem_dir_len = strlen(path) + 10;
    repo->stem_dir = malloc(stem_dir_len);
    if (!repo->stem_dir) {
        free(repo->path);
        free(repo);
        return STEM_ERROR_MEMORY;
    }
    
    if (bare) {
        snprintf(repo->stem_dir, stem_dir_len, "%s", path);
    } else {
        snprintf(repo->stem_dir, stem_dir_len, "%s%s.stem", path, STEM_PATH_SEP_STR);
    }
    
    repo->is_bare = bare;
    
    /* Create repository structure */
    stem_error_t err = create_repo_structure(repo->stem_dir);
    if (err != STEM_OK) {
        stem_repository_free(repo);
        return err;
    }
    
    /* Create default config */
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s%sconfig", repo->stem_dir, STEM_PATH_SEP_STR);
    
    err = stem_config_open(config_path, &repo->config);
    if (err != STEM_OK) {
        stem_repository_free(repo);
        return err;
    }
    
    /* Set default config values */
    stem_config_set_bool(repo->config, "core", "bare", bare);
    stem_config_set_int(repo->config, "core", "repositoryformatversion", 0);
    stem_config_save(repo->config);
    
    *repo_out = repo;
    return STEM_OK;
}

stem_error_t stem_repository_open(const char *path, stem_repository_t **repo_out) {
    if (!path || !repo_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    stem_repository_t *repo = calloc(1, sizeof(stem_repository_t));
    if (!repo) {
        return STEM_ERROR_MEMORY;
    }
    
    repo->path = strdup(path);
    if (!repo->path) {
        free(repo);
        return STEM_ERROR_MEMORY;
    }
    
    /* Find .stem directory */
    size_t stem_dir_len = strlen(path) + 10;
    repo->stem_dir = malloc(stem_dir_len);
    if (!repo->stem_dir) {
        free(repo->path);
        free(repo);
        return STEM_ERROR_MEMORY;
    }
    
    snprintf(repo->stem_dir, stem_dir_len, "%s%s.stem", path, STEM_PATH_SEP_STR);
    
    if (!is_directory(repo->stem_dir)) {
        /* Try as bare repository */
        snprintf(repo->stem_dir, stem_dir_len, "%s", path);
        if (!is_directory(repo->stem_dir)) {
            stem_repository_free(repo);
            return STEM_ERROR_NOT_FOUND;
        }
        repo->is_bare = true;
    }
    
    /* Open config */
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s%sconfig", repo->stem_dir, STEM_PATH_SEP_STR);
    
    stem_error_t err = stem_config_open(config_path, &repo->config);
    if (err != STEM_OK) {
        stem_repository_free(repo);
        return err;
    }
    
    *repo_out = repo;
    return STEM_OK;
}

void stem_repository_free(stem_repository_t *repo) {
    if (!repo) return;
    
    free(repo->path);
    free(repo->stem_dir);
    
    if (repo->config) {
        stem_config_free(repo->config);
    }
    
    if (repo->index) {
        stem_index_free(repo->index);
    }
    
    free(repo);
}

stem_error_t stem_repository_discover(const char *start_path, char **repo_path_out) {
    if (!start_path || !repo_path_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    char current[1024];
    strncpy(current, start_path, sizeof(current) - 1);
    current[sizeof(current) - 1] = '\0';
    
    while (1) {
        char stem_path[1024];
        snprintf(stem_path, sizeof(stem_path), "%s%s.stem", current, STEM_PATH_SEP_STR);
        
        if (is_directory(stem_path)) {
            *repo_path_out = strdup(current);
            return *repo_path_out ? STEM_OK : STEM_ERROR_MEMORY;
        }
        
        /* Move to parent directory */
        char *last_sep = strrchr(current, STEM_PATH_SEP);
        if (!last_sep || last_sep == current) {
            break;
        }
        *last_sep = '\0';
    }
    
    return STEM_ERROR_NOT_FOUND;
}

bool stem_repository_is_valid(const stem_repository_t *repo) {
    return repo && repo->stem_dir && is_directory(repo->stem_dir);
}

const char* stem_repository_path(const stem_repository_t *repo) {
    return repo ? repo->path : NULL;
}

const char* stem_repository_stem_dir(const stem_repository_t *repo) {
    return repo ? repo->stem_dir : NULL;
}
