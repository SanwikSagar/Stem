#include "commands.h"
#include "stem/stem.h"
#include "output.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

/* Get current branch name */
static int get_current_branch(char *branch, size_t size) {
    FILE *head = fopen(".stem/HEAD", "r");
    if (!head) {
        return -1;
    }
    
    char line[512];
    if (fgets(line, sizeof(line), head)) {
        /* Remove newline */
        line[strcspn(line, "\r\n")] = 0;
        
        /* Check if detached HEAD (direct commit hash) */
        if (strncmp(line, "ref: refs/heads/", 16) == 0) {
            strncpy(branch, line + 16, size - 1);
            branch[size - 1] = '\0';
            fclose(head);
            return 0;
        }
    }
    
    fclose(head);
    return -1;
}

/* List all branches */
static int list_branches() {
    char current[256] = {0};
    get_current_branch(current, sizeof(current));
    
    printf("\n");
    printf("Branches:\n");
    printf("\n");
    
#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(".stem/refs/heads/*", &find_data);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        print_warning("No branches found");
        printf("\n");
        printf("Create a branch with: stem branch <name>\n");
        return 0;
    }
    
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || 
            strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        
        if (strcmp(find_data.cFileName, current) == 0) {
            printf("  * ");
            print_success("%s (current)", find_data.cFileName);
        } else {
            printf("    %s\n", find_data.cFileName);
        }
    } while (FindNextFileA(hFind, &find_data));
    
    FindClose(hFind);
#else
    DIR *dir = opendir(".stem/refs/heads");
    if (!dir) {
        print_warning("No branches found");
        printf("\n");
        printf("Create a branch with: stem branch <name>\n");
        return 0;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        if (strcmp(entry->d_name, current) == 0) {
            printf("  * ");
            print_success("%s (current)", entry->d_name);
        } else {
            printf("    %s\n", entry->d_name);
        }
    }
    
    closedir(dir);
#endif
    
    printf("\n");
    return 0;
}

/* Create a new branch */
static int create_branch(const char *name) {
    /* Validate branch name */
    if (strlen(name) == 0) {
        print_error("Branch name cannot be empty");
        return 1;
    }
    
    /* Check for invalid characters */
    for (const char *p = name; *p; p++) {
        if (*p == '/' || *p == '\\' || *p == ' ' || *p == ':' || *p == '*' || *p == '?') {
            print_error("Invalid branch name: contains illegal characters");
            return 1;
        }
    }
    
    /* Check if branch already exists */
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".stem/refs/heads/%s", name);
    
    FILE *check = fopen(branch_path, "r");
    if (check) {
        fclose(check);
        print_error("Branch '%s' already exists", name);
        return 1;
    }
    
    /* Get current HEAD commit */
    FILE *head = fopen(".stem/HEAD", "r");
    if (!head) {
        print_error("Not a STEM repository");
        return 1;
    }
    
    char current_ref[512];
    if (!fgets(current_ref, sizeof(current_ref), head)) {
        fclose(head);
        print_error("Could not read HEAD");
        return 1;
    }
    fclose(head);
    
    current_ref[strcspn(current_ref, "\r\n")] = 0;
    
    /* Get commit hash from current branch */
    char commit_hash[128] = {0};
    
    if (strncmp(current_ref, "ref: ", 5) == 0) {
        /* HEAD points to a branch */
        FILE *branch_file = fopen(current_ref + 5, "r");
        if (branch_file) {
            if (fgets(commit_hash, sizeof(commit_hash), branch_file)) {
                commit_hash[strcspn(commit_hash, "\r\n")] = 0;
            }
            fclose(branch_file);
        }
    } else {
        /* Detached HEAD - copy safely */
        size_t len = strlen(current_ref);
        if (len > sizeof(commit_hash) - 1) len = sizeof(commit_hash) - 1;
        memcpy(commit_hash, current_ref, len);
        commit_hash[len] = '\0';
    }
    
    /* If no commit yet, use empty */
    if (strlen(commit_hash) == 0) {
        strcpy(commit_hash, "0000000");
    }
    
    /* Create new branch file */
    FILE *new_branch = fopen(branch_path, "w");
    if (!new_branch) {
        print_error("Could not create branch");
        return 1;
    }
    
    fprintf(new_branch, "%s\n", commit_hash);
    fclose(new_branch);
    
    printf("\n");
    print_success("Created branch '%s'", name);
    printf("\n");
    printf("Switch to it with: stem switch %s\n", name);
    printf("\n");
    
    return 0;
}

/* Delete a branch */
static int delete_branch(const char *name) {
    /* Check if it's the current branch */
    char current[256];
    if (get_current_branch(current, sizeof(current)) == 0) {
        if (strcmp(current, name) == 0) {
            print_error("Cannot delete the current branch");
            printf("\n");
            printf("Switch to another branch first: stem switch <branch>\n");
            return 1;
        }
    }
    
    /* Check if branch exists */
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".stem/refs/heads/%s", name);
    
    FILE *check = fopen(branch_path, "r");
    if (!check) {
        print_error("Branch '%s' does not exist", name);
        return 1;
    }
    fclose(check);
    
    /* Delete the branch */
    if (remove(branch_path) != 0) {
        print_error("Could not delete branch");
        return 1;
    }
    
    printf("\n");
    print_success("Deleted branch '%s'", name);
    printf("\n");
    
    return 0;
}

/* Branch command */
int cmd_branch(int argc, char **argv) {
    const char *branch_name = NULL;
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
            printf("Usage: stem branch [options] [<name>]\n");
            printf("\n");
            printf("Manage branches\n");
            printf("\n");
            printf("Options:\n");
            printf("  (no args)         List all branches\n");
            printf("  <name>            Create new branch\n");
            printf("  -d, --delete <name>  Delete branch\n");
            printf("  -l, --list        List branches\n");
            printf("  --help            Show this help\n");
            printf("\n");
            printf("Examples:\n");
            printf("  stem branch              # List branches\n");
            printf("  stem branch feature-x    # Create branch\n");
            printf("  stem branch -d old-feature  # Delete branch\n");
            printf("\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            branch_name = argv[i];
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
    
    /* Ensure refs/heads directory exists */
    mkdir(".stem/refs", 0755);
    mkdir(".stem/refs/heads", 0755);
    
    /* Execute command */
    if (delete_mode) {
        if (!branch_name) {
            print_error("Please specify a branch name to delete");
            printf("\n");
            printf("Usage: stem branch -d <name>\n");
            return 1;
        }
        return delete_branch(branch_name);
    }
    else if (branch_name) {
        return create_branch(branch_name);
    }
    else {
        return list_branches();
    }
}

/* Switch branch command */
int cmd_switch(int argc, char **argv) {
    const char *branch_name = NULL;
    int create_mode = 0;
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--create") == 0) {
            create_mode = 1;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("\n");
            printf("Usage: stem switch [options] <branch>\n");
            printf("\n");
            printf("Switch to a different branch\n");
            printf("\n");
            printf("Options:\n");
            printf("  -c, --create      Create branch if it doesn't exist\n");
            printf("  --help            Show this help\n");
            printf("\n");
            printf("Examples:\n");
            printf("  stem switch main           # Switch to main\n");
            printf("  stem switch -c feature-x   # Create and switch\n");
            printf("\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            branch_name = argv[i];
        }
    }
    
    if (!branch_name) {
        print_error("Please specify a branch name");
        printf("\n");
        printf("Usage: stem switch <branch>\n");
        printf("\n");
        printf("Available branches:\n");
        list_branches();
        return 1;
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
    
    /* Check if branch exists */
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".stem/refs/heads/%s", branch_name);
    
    FILE *branch_file = fopen(branch_path, "r");
    if (!branch_file) {
        if (create_mode) {
            /* Create the branch first */
            printf("\n");
            print_info("Creating new branch '%s'...", branch_name);
            if (create_branch(branch_name) != 0) {
                return 1;
            }
        } else {
            print_error("Branch '%s' does not exist", branch_name);
            printf("\n");
            printf("Create it with: stem branch %s\n", branch_name);
            printf("Or: stem switch -c %s\n", branch_name);
            return 1;
        }
    } else {
        fclose(branch_file);
    }
    
    /* Update HEAD to point to new branch */
    FILE *new_head = fopen(".stem/HEAD", "w");
    if (!new_head) {
        print_error("Could not update HEAD");
        return 1;
    }
    
    fprintf(new_head, "ref: .stem/refs/heads/%s\n", branch_name);
    fclose(new_head);
    
    printf("\n");
    print_success("Switched to branch '%s'", branch_name);
    printf("\n");
    
    return 0;
}
