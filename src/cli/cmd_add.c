#include "commands.h"
#include "stem/stem.h"
#include "output.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd

/* Global ignore patterns storage */
#define MAX_IGNORE_PATTERNS 256
static char *ignore_patterns[MAX_IGNORE_PATTERNS];
static int ignore_pattern_count = 0;

/* Load .stemignore file */
static void load_stemignore(void) {
    FILE *f = fopen(".stemignore", "r");
    if (!f) {
        /* No .stemignore file, that's okay */
        return;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), f) && ignore_pattern_count < MAX_IGNORE_PATTERNS) {
        /* Remove newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        
        /* Skip empty lines and comments */
        if (len == 0 || line[0] == '#') {
            continue;
        }
        
        /* Trim leading whitespace */
        char *start = line;
        while (*start && isspace((unsigned char)*start)) {
            start++;
        }
        
        if (*start == '\0') {
            continue;
        }
        
        /* Store pattern */
        ignore_patterns[ignore_pattern_count] = _strdup(start);
        if (ignore_patterns[ignore_pattern_count]) {
            ignore_pattern_count++;
        }
    }
    
    fclose(f);
}

/* Free ignore patterns */
static void free_stemignore(void) {
    for (int i = 0; i < ignore_pattern_count; i++) {
        free(ignore_patterns[i]);
        ignore_patterns[i] = NULL;
    }
    ignore_pattern_count = 0;
}

/* Check if a path matches an ignore pattern */
static int should_ignore(const char *path) {
    /* Always ignore .stem directory */
    if (strstr(path, "\\.stem\\") || strstr(path, "\\.stem") == path + strlen(path) - 5) {
        return 1;
    }
    
    for (int i = 0; i < ignore_pattern_count; i++) {
        const char *pattern = ignore_patterns[i];
        int pattern_len = strlen(pattern);
        
        /* Pattern ends with / - it's a directory pattern */
        if (pattern[pattern_len - 1] == '/') {
            char dir_pattern[512];
            snprintf(dir_pattern, sizeof(dir_pattern), "\\%.*s\\", pattern_len - 1, pattern);
            if (strstr(path, dir_pattern)) {
                return 1;
            }
            /* Also check if path ends with directory name */
            snprintf(dir_pattern, sizeof(dir_pattern), "\\%.*s", pattern_len - 1, pattern);
            if (strstr(path, dir_pattern)) {
                return 1;
            }
        }
        /* Pattern starts with * - it's an extension pattern */
        else if (pattern[0] == '*') {
            const char *ext = pattern + 1;
            if (strlen(path) >= strlen(ext)) {
                if (strcmp(path + strlen(path) - strlen(ext), ext) == 0) {
                    return 1;
                }
            }
        }
        /* Exact filename match */
        else {
            /* Check if filename appears anywhere in path */
            char search[512];
            snprintf(search, sizeof(search), "\\%s", pattern);
            if (strstr(path, search)) {
                return 1;
            }
            /* Also check at the end of path */
            if (strlen(path) >= strlen(pattern)) {
                if (strcmp(path + strlen(path) - strlen(pattern), pattern) == 0) {
                    return 1;
                }
            }
        }
    }
    
    return 0;
}

static int add_directory_recursive(const char *path, int *file_count, FILE *index_file) {
    WIN32_FIND_DATAA find_data;
    HANDLE hFind;
    char search_path[MAX_PATH];
    char full_path[MAX_PATH];
    
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    
    hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    do {
        /* Skip . and .. */
        if (strcmp(find_data.cFileName, ".") == 0 || 
            strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        
        /* Build full path */
        snprintf(full_path, sizeof(full_path), "%s\\%s", path, find_data.cFileName);
        
        /* Check if should be ignored */
        if (should_ignore(full_path)) {
            continue;
        }
        
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            /* Recursively add directory */
            add_directory_recursive(full_path, file_count, index_file);
        } else {
            /* Add file */
            printf("  + %s\n", find_data.cFileName);
            fprintf(index_file, "%s\n", full_path);
            (*file_count)++;
        }
    } while (FindNextFileA(hFind, &find_data));
    
    FindClose(hFind);
    return 1;
}

static int add_single_file(const char *filename, int *file_count, FILE *index_file) {
    /* Check if file exists */
    DWORD attr = GetFileAttributesA(filename);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        print_error("File not found: %s", filename);
        return 0;
    }
    
    /* Check if should be ignored */
    if (should_ignore(filename)) {
        return 1; /* Silently skip ignored files */
    }
    
    if (attr & FILE_ATTRIBUTE_DIRECTORY) {
        /* It's a directory, add recursively */
        return add_directory_recursive(filename, file_count, index_file);
    } else {
        /* It's a file */
        printf("  + %s\n", filename);
        fprintf(index_file, "%s\n", filename);
        (*file_count)++;
        return 1;
    }
}

#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/* Global ignore patterns storage */
#define MAX_IGNORE_PATTERNS 256
static char *ignore_patterns[MAX_IGNORE_PATTERNS];
static int ignore_pattern_count = 0;

/* Load .stemignore file */
static void load_stemignore(void) {
    FILE *f = fopen(".stemignore", "r");
    if (!f) {
        return;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), f) && ignore_pattern_count < MAX_IGNORE_PATTERNS) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        
        if (len == 0 || line[0] == '#') {
            continue;
        }
        
        char *start = line;
        while (*start && isspace((unsigned char)*start)) {
            start++;
        }
        
        if (*start == '\0') {
            continue;
        }
        
        ignore_patterns[ignore_pattern_count] = strdup(start);
        if (ignore_patterns[ignore_pattern_count]) {
            ignore_pattern_count++;
        }
    }
    
    fclose(f);
}

/* Free ignore patterns */
static void free_stemignore(void) {
    for (int i = 0; i < ignore_pattern_count; i++) {
        free(ignore_patterns[i]);
        ignore_patterns[i] = NULL;
    }
    ignore_pattern_count = 0;
}

/* Check if a path matches an ignore pattern */
static int should_ignore(const char *path) {
    if (strstr(path, "/.stem/") || strstr(path, "/.stem") == path + strlen(path) - 6) {
        return 1;
    }
    
    for (int i = 0; i < ignore_pattern_count; i++) {
        const char *pattern = ignore_patterns[i];
        int pattern_len = strlen(pattern);
        
        if (pattern[pattern_len - 1] == '/') {
            char dir_pattern[512];
            snprintf(dir_pattern, sizeof(dir_pattern), "/%.*s/", pattern_len - 1, pattern);
            if (strstr(path, dir_pattern)) {
                return 1;
            }
            snprintf(dir_pattern, sizeof(dir_pattern), "/%.*s", pattern_len - 1, pattern);
            if (strstr(path, dir_pattern)) {
                return 1;
            }
        }
        else if (pattern[0] == '*') {
            const char *ext = pattern + 1;
            if (strlen(path) >= strlen(ext)) {
                if (strcmp(path + strlen(path) - strlen(ext), ext) == 0) {
                    return 1;
                }
            }
        }
        else {
            char search[512];
            snprintf(search, sizeof(search), "/%s", pattern);
            if (strstr(path, search)) {
                return 1;
            }
            if (strlen(path) >= strlen(pattern)) {
                if (strcmp(path + strlen(path) - strlen(pattern), pattern) == 0) {
                    return 1;
                }
            }
        }
    }
    
    return 0;
}

static int add_directory_recursive(const char *path, int *file_count, FILE *index_file) {
    DIR *dir;
    struct dirent *entry;
    char full_path[PATH_MAX];
    struct stat st;
    
    dir = opendir(path);
    if (!dir) {
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        /* Skip . and .. */
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        /* Check if should be ignored */
        if (should_ignore(full_path)) {
            continue;
        }
        
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                add_directory_recursive(full_path, file_count, index_file);
            } else {
                printf("  + %s\n", entry->d_name);
                fprintf(index_file, "%s\n", full_path);
                (*file_count)++;
            }
        }
    }
    
    closedir(dir);
    return 1;
}

static int add_single_file(const char *filename, int *file_count, FILE *index_file) {
    struct stat st;
    
    if (stat(filename, &st) != 0) {
        print_error("File not found: %s", filename);
        return 0;
    }
    
    /* Check if should be ignored */
    if (should_ignore(filename)) {
        return 1; /* Silently skip ignored files */
    }
    
    if (S_ISDIR(st.st_mode)) {
        return add_directory_recursive(filename, file_count, index_file);
    } else {
        printf("  + %s\n", filename);
        fprintf(index_file, "%s\n", filename);
        (*file_count)++;
        return 1;
    }
}
#endif

int cmd_add(int argc, char **argv) {
    int total_files = 0;
    
    if (argc < 2) {
        print_error("Usage: stem add <files...>");
        printf("\nExamples:\n");
        printf("  stem add file.txt          Add single file\n");
        printf("  stem add .                 Add all files in current directory\n");
        printf("  stem add src/              Add all files in src/ directory\n");
        printf("  stem add *.c               Add all .c files\n");
        return 1;
    }
    
    /* Check if .stemignore exists, if not, prompt to create it */
    FILE *check_ignore = fopen(".stemignore", "r");
    if (!check_ignore) {
        printf("\n");
        printf("================================================================\n");
        printf("  .stemignore File Not Found\n");
        printf("================================================================\n");
        printf("\n");
        printf("A .stemignore file helps you exclude unwanted files from commits:\n");
        printf("\n");
        printf("  [*] Build Outputs     *.exe, *.o, *.dll, bin/, obj/\n");
        printf("  [*] Temporary Files   *.tmp, *.log, *.cache, *.bak\n");
        printf("  [*] IDE Files         .vscode/, .idea/, .vs/\n");
        printf("  [*] Dependencies      node_modules/, vendor/, packages/\n");
        printf("  [*] Test Files        *.test, *.dat, test/, tests/\n");
        printf("  [*] Secrets           *.key, .env, passwords, tokens\n");
        printf("\n");
        printf("This prevents uploading gigabytes of garbage to Google Drive.\n");
        printf("\n");
        printf("================================================================\n");
        printf("\n");
        printf("Create .stemignore file now? [Y/n]: ");
        
        char response[10];
        if (fgets(response, sizeof(response), stdin)) {
            if (response[0] == '\n' || response[0] == 'y' || response[0] == 'Y') {
                /* Copy template from STEM installation */
                FILE *template = fopen(".stemignore.template", "r");
                if (template) {
                    FILE *ignore = fopen(".stemignore", "w");
                    if (ignore) {
                        char buffer[1024];
                        while (fgets(buffer, sizeof(buffer), template)) {
                            fputs(buffer, ignore);
                        }
                        fclose(ignore);
                        printf("\n");
                        print_success("Created .stemignore file");
                        printf("\n");
                        printf("Next steps:\n");
                        printf("  1. Open .stemignore in your text editor\n");
                        printf("  2. Review the patterns (uncomment # lines you need)\n");
                        printf("  3. Run 'stem add .' again to stage your files\n");
                        printf("\n");
                        printf("================================================================\n");
                        printf("\n");
                    }
                    fclose(template);
                    return 0;
                } else {
                    printf("\n");
                    print_warning("Could not find .stemignore template");
                    printf("\n");
                    printf("Create .stemignore manually or continue without it.\n");
                    printf("Copy .stemignore.template into this repository and try again.\n");
                    printf("\n");
                }
            } else {
                printf("\n");
                print_warning("Proceeding without .stemignore");
                printf("All files will be added (including build outputs and temp files)\n");
                printf("\n");
            }
        }
    } else {
        fclose(check_ignore);
    }
    
    /* Load .stemignore patterns */
    load_stemignore();
    
    /* Open index file for writing */
    FILE *index_file = fopen(".stem/index", "w");
    if (!index_file) {
        print_error("Failed to open index file. Run 'stem init' first.");
        free_stemignore();
        return 1;
    }
    
    printf("\n");
    printf("================================================================\n");
    printf("  Staging Files for Commit\n");
    printf("================================================================\n");
    printf("\n");
    
    if (ignore_pattern_count > 0) {
        printf("[*] Loaded %d ignore pattern(s) from .stemignore\n", ignore_pattern_count);
        printf("\n");
    }
    
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        
        /* Handle current directory */
        if (strcmp(arg, ".") == 0) {
            char cwd[MAX_PATH];
            if (getcwd(cwd, sizeof(cwd))) {
                add_directory_recursive(cwd, &total_files, index_file);
            }
        } else {
            add_single_file(arg, &total_files, index_file);
        }
    }
    
    fclose(index_file);
    free_stemignore();
    
    printf("\n");
    printf("================================================================\n");
    printf("  Staging Complete\n");
    printf("================================================================\n");
    printf("\n");
    print_success("%d file(s) staged for commit", total_files);
    printf("\n");
    
    if (ignore_pattern_count > 0) {
        print_info("Files matching .stemignore patterns were excluded");
    }
    
    printf("\n");
    printf("Next step: stem commit -m \"Your commit message\"\n");
    printf("\n");
    
    return 0;
}
