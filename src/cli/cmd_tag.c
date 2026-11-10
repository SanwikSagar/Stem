#include "commands.h"
#include "stem/stem.h"
#include "output.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

/* Get current commit hash */
static int get_current_commit(char *hash, size_t size) {
    if (!hash || size == 0) {
        return -1;
    }

    FILE *head = fopen(".stem/HEAD", "r");
    if (!head) {
        return -1;
    }
    
    char line[512];
    if (!fgets(line, sizeof(line), head)) {
        fclose(head);
        return -1;
    }
    fclose(head);
    
    line[strcspn(line, "\r\n")] = 0;
    
    /* If HEAD points to a branch */
    if (strncmp(line, "ref: ", 5) == 0) {
        FILE *branch_file = fopen(line + 5, "r");
        if (!branch_file) {
            return -1;
        }
        
        if (!fgets(hash, size, branch_file)) {
            fclose(branch_file);
            return -1;
        }
        fclose(branch_file);
        
        hash[strcspn(hash, "\r\n")] = 0;
        return 0;
    }
    
    /* Detached HEAD */
    {
        size_t line_len = strlen(line);
        size_t copy_len = (line_len < (size - 1)) ? line_len : (size - 1);
        memcpy(hash, line, copy_len);
        hash[copy_len] = '\0';
    }
    return 0;
}

/* List all tags */
static int list_tags() {
    printf("\n");
    printf("Tags:\n");
    printf("\n");
    
    int count = 0;
    
#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(".stem/refs/tags/*", &find_data);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        print_warning("No tags found");
        printf("\n");
        printf("Create a tag with: stem tag <name>\n");
        return 0;
    }
    
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || 
            strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        
        char tag_path[512];
        snprintf(tag_path, sizeof(tag_path), ".stem/refs/tags/%s", find_data.cFileName);
        
        FILE *tag_file = fopen(tag_path, "r");
        if (tag_file) {
            char commit_hash[128];
            if (fgets(commit_hash, sizeof(commit_hash), tag_file)) {
                commit_hash[strcspn(commit_hash, "\r\n")] = 0;
                printf("  %-20s -> %s\n", find_data.cFileName, commit_hash);
                count++;
            }
            fclose(tag_file);
        }
    } while (FindNextFileA(hFind, &find_data));
    
    FindClose(hFind);
#else
    DIR *dir = opendir(".stem/refs/tags");
    if (!dir) {
        print_warning("No tags found");
        printf("\n");
        printf("Create a tag with: stem tag <name>\n");
        return 0;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char tag_path[512];
        snprintf(tag_path, sizeof(tag_path), ".stem/refs/tags/%s", entry->d_name);
        
        FILE *tag_file = fopen(tag_path, "r");
        if (tag_file) {
            char commit_hash[128];
            if (fgets(commit_hash, sizeof(commit_hash), tag_file)) {
                commit_hash[strcspn(commit_hash, "\r\n")] = 0;
                printf("  %-20s -> %s\n", entry->d_name, commit_hash);
                count++;
            }
            fclose(tag_file);
        }
    }
    
    closedir(dir);
#endif
    
    if (count == 0) {
        print_warning("No tags found");
        printf("\n");
        printf("Create a tag with: stem tag <name>\n");
    }
    
    printf("\n");
    return 0;
}

/* Create a tag */
static int create_tag(const char *name) {
    /* Validate tag name */
    if (strlen(name) == 0) {
        print_error("Tag name cannot be empty");
        return 1;
    }
    
    /* Check for invalid characters */
    for (const char *p = name; *p; p++) {
        if (*p == '/' || *p == '\\' || *p == ' ' || *p == ':' || *p == '*' || *p == '?') {
            print_error("Invalid tag name: contains illegal characters");
            return 1;
        }
    }
    
    /* Check if tag already exists */
    char tag_path[512];
    snprintf(tag_path, sizeof(tag_path), ".stem/refs/tags/%s", name);
    
    FILE *check = fopen(tag_path, "r");
    if (check) {
        fclose(check);
        print_error("Tag '%s' already exists", name);
        return 1;
    }
    
    /* Get current commit */
    char commit_hash[128];
    if (get_current_commit(commit_hash, sizeof(commit_hash)) != 0) {
        print_error("Could not get current commit");
        return 1;
    }
    
    /* Create tag file */
    FILE *tag_file = fopen(tag_path, "w");
    if (!tag_file) {
        print_error("Could not create tag");
        return 1;
    }
    
    fprintf(tag_file, "%s\n", commit_hash);
    fclose(tag_file);
    
    printf("\n");
    print_success("Created tag '%s' at commit %s", name, commit_hash);
    printf("\n");
    
    return 0;
}

/* Delete a tag */
static int delete_tag(const char *name) {
    /* Check if tag exists */
    char tag_path[512];
    snprintf(tag_path, sizeof(tag_path), ".stem/refs/tags/%s", name);
    
    FILE *check = fopen(tag_path, "r");
    if (!check) {
        print_error("Tag '%s' does not exist", name);
        return 1;
    }
    fclose(check);
    
    /* Delete the tag */
    if (remove(tag_path) != 0) {
        print_error("Could not delete tag");
        return 1;
    }
    
    printf("\n");
    print_success("Deleted tag '%s'", name);
    printf("\n");
    
    return 0;
}

/* Tag command */
int cmd_tag(int argc, char **argv) {
    const char *tag_name = NULL;
    int delete_mode = 0;
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delete") == 0) {
            delete_mode = 1;
        }
        else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            /* List mode - just continue to list */
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("\n");
            printf("Usage: stem tag [options] [<name>]\n");
            printf("\n");
            printf("Manage tags\n");
            printf("\n");
            printf("Options:\n");
            printf("  (no args)         List all tags\n");
            printf("  <name>            Create new tag at current commit\n");
            printf("  -d, --delete <name>  Delete tag\n");
            printf("  -l, --list        List tags\n");
            printf("  --help            Show this help\n");
            printf("\n");
            printf("Examples:\n");
            printf("  stem tag                  # List tags\n");
            printf("  stem tag v1.0.0           # Create tag\n");
            printf("  stem tag -d old-tag       # Delete tag\n");
            printf("\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            tag_name = argv[i];
        }
    }
    
    /* Check if in a repository */
    FILE *head = fopen(".stem/HEAD", "r");
    if (!head) {
        print_error("Not a STEM repository");
        printf("\n");
        printf("Initialize with: stem init\n");
        return 1;
    }
    fclose(head);
    
    /* Ensure refs/tags directory exists */
    mkdir(".stem/refs", 0755);
    mkdir(".stem/refs/tags", 0755);
    
    /* Execute command */
    if (delete_mode) {
        if (!tag_name) {
            print_error("Please specify a tag name to delete");
            printf("\n");
            printf("Usage: stem tag -d <name>\n");
            return 1;
        }
        return delete_tag(tag_name);
    }
    else if (tag_name) {
        return create_tag(tag_name);
    }
    else {
        return list_tags();
    }
}
