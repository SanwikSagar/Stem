#include "commands.h"
#include "stem/stem.h"
#include "output.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

/* Read commit file list */
static int get_commit_files(const char *commit_id, char ***files, int *count) {
    char commit_path[512];
    snprintf(commit_path, sizeof(commit_path), ".stem/objects/commits/%s", commit_id);
    
    FILE *f = fopen(commit_path, "r");
    if (!f) return -1;
    
    /* Count files after "--- files ---" marker */
    char line[2048];
    int in_files = 0;
    *count = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "--- files ---")) {
            in_files = 1;
            continue;
        }
        if (in_files) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) > 0) {
                (*count)++;
            }
        }
    }
    
    if (*count == 0) {
        fclose(f);
        return 0;
    }
    
    /* Allocate and read files */
    *files = (char **)malloc(*count * sizeof(char *));
    if (!*files) {
        fclose(f);
        return -1;
    }
    
    rewind(f);
    in_files = 0;
    int idx = 0;
    
    while (fgets(line, sizeof(line), f) && idx < *count) {
        if (strstr(line, "--- files ---")) {
            in_files = 1;
            continue;
        }
        if (in_files) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) > 0) {
                (*files)[idx++] = strdup(line);
            }
        }
    }
    
    fclose(f);
    return 0;
}

/* Create conflict markers in a file */
static int mark_conflict(const char *file_path, const char *current_branch, const char *merge_branch) {
    char temp_path[2048];
    snprintf(temp_path, sizeof(temp_path), "%s.conflict", file_path);
    
    /* Read both versions */
    FILE *current = fopen(file_path, "r");
    char current_obj[2048];
    snprintf(current_obj, sizeof(current_obj), ".stem/objects/%s", file_path);
    FILE *original = fopen(current_obj, "r");
    
    FILE *output = fopen(temp_path, "w");
    if (!output) return -1;
    
    /* Write conflict markers */
    fprintf(output, "<<<<<<< HEAD (%s)\n", current_branch);
    
    if (current) {
        char line[2048];
        while (fgets(line, sizeof(line), current)) {
            fputs(line, output);
        }
        fclose(current);
    }
    
    fprintf(output, "=======\n");
    
    if (original) {
        char line[2048];
        while (fgets(line, sizeof(line), original)) {
            fputs(line, output);
        }
        fclose(original);
    }
    
    fprintf(output, ">>>>>>> %s\n", merge_branch);
    
    fclose(output);
    
    /* Replace original with conflict-marked version */
    remove(file_path);
    rename(temp_path, file_path);
    
    return 0;
}

/* Find common ancestor commit (simplified - just finds parent) */
static int find_merge_base(const char *commit1, const char *commit2 __attribute__((unused)), 
                           char *base, size_t size) {
    /* For now, use commit1's parent as merge base */
    /* TODO: Implement proper merge base finding algorithm using commit2 */
    
    char commit_path[512];
    snprintf(commit_path, sizeof(commit_path), ".stem/objects/commits/%s", commit1);
    
    FILE *f = fopen(commit_path, "r");
    if (!f) return -1;
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "parent ", 7) == 0) {
            line[strcspn(line, "\r\n")] = 0;
            size_t len = strlen(line + 7);
            if (len > size - 1) len = size - 1;
            memcpy(base, line + 7, len);
            base[len] = '\0';
            fclose(f);
            return 0;
        }
    }
    
    fclose(f);
    return -1;
}

/* Perform 3-way merge on a single file */
static int merge_file_3way(const char *file_path, const char *base_commit, 
                           const char *current_commit __attribute__((unused)), 
                           const char *merge_commit __attribute__((unused))) {
    /* File paths in different commits */
    /* TODO: Use current_commit and merge_commit for actual 3-way comparison */
    char base_file[2048], current_file[2048], merge_file[2048];
    snprintf(base_file, sizeof(base_file), ".stem/objects/%s.%s", file_path, base_commit);
    snprintf(current_file, sizeof(current_file), ".stem/objects/%s", file_path);
    snprintf(merge_file, sizeof(merge_file), ".stem/objects/%s.%s", file_path, base_commit);
    
    /* For now, just check if files are identical or conflicting */
    FILE *f_current = fopen(current_file, "rb");
    FILE *f_merge = fopen(merge_file, "rb");
    
    if (!f_current || !f_merge) {
        if (f_current) fclose(f_current);
        if (f_merge) fclose(f_merge);
        return 0;  /* One side doesn't have the file */
    }
    
    /* Simple byte comparison */
    int conflict = 0;
    char buf1[8192], buf2[8192];
    size_t n1, n2;
    
    while (1) {
        n1 = fread(buf1, 1, sizeof(buf1), f_current);
        n2 = fread(buf2, 1, sizeof(buf2), f_merge);
        
        if (n1 != n2 || memcmp(buf1, buf2, n1) != 0) {
            conflict = 1;
            break;
        }
        
        if (n1 == 0) break;
    }
    
    fclose(f_current);
    fclose(f_merge);
    
    return conflict ? -1 : 0;
}

/* Get current branch name */
static int get_current_branch(char *branch, size_t size) {
    FILE *head = fopen(".stem/HEAD", "r");
    if (!head) {
        return -1;
    }
    
    char line[512];
    if (fgets(line, sizeof(line), head)) {
        line[strcspn(line, "\r\n")] = 0;
        
        if (strncmp(line, "ref: refs/heads/", 16) == 0) {
            strncpy(branch, line + 16, size - 1);
            branch[size - 1] = '\0';
            fclose(head);
            return 0;
        } else if (strncmp(line, "ref: .stem/refs/heads/", 22) == 0) {
            strncpy(branch, line + 22, size - 1);
            branch[size - 1] = '\0';
            fclose(head);
            return 0;
        }
    }
    
    fclose(head);
    return -1;
}

/* Get commit hash for a branch */
static int get_branch_commit(const char *branch, char *commit, size_t size) {
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".stem/refs/heads/%s", branch);
    
    FILE *branch_file = fopen(branch_path, "r");
    if (!branch_file) {
        return -1;
    }
    
    if (fgets(commit, size, branch_file)) {
        commit[strcspn(commit, "\r\n")] = 0;
        fclose(branch_file);
        return 0;
    }
    
    fclose(branch_file);
    return -1;
}

/* Simple fast-forward merge */
static int fast_forward_merge(const char *from_branch __attribute__((unused)), 
                              const char *to_commit) {
    char current_branch[256];
    if (get_current_branch(current_branch, sizeof(current_branch)) != 0) {
        print_error("Could not determine current branch");
        return 1;
    }
    
    /* Update current branch to point to new commit */
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".stem/refs/heads/%s", current_branch);
    
    FILE *branch_file = fopen(branch_path, "w");
    if (!branch_file) {
        print_error("Could not update branch");
        return 1;
    }
    
    fprintf(branch_file, "%s\n", to_commit);
    fclose(branch_file);
    
    printf("\n");
    print_success("Fast-forward merge successful");
    printf("\n");
    printf("Branch '%s' now points to commit %s\n", current_branch, to_commit);
    printf("  (merged from %s)\n", from_branch);
    printf("\n");
    
    return 0;
}

static int confirm_merge_action(const char *prompt) {
    char answer[16];
    printf("%s [y/N]: ", prompt);
    if (!fgets(answer, sizeof(answer), stdin)) {
        return 0;
    }
    return (answer[0] == 'y' || answer[0] == 'Y');
}

/* Merge command */
int cmd_merge(int argc, char **argv) {
    const char *branch_name = NULL;
    int abort_merge = 0;
    int continue_merge = 0;
    int dry_run = 0;
    int assume_yes = 0;
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--abort") == 0) {
            abort_merge = 1;
        }
        else if (strcmp(argv[i], "--continue") == 0) {
            continue_merge = 1;
        }
        else if (strcmp(argv[i], "--dry-run") == 0) {
            dry_run = 1;
        }
        else if (strcmp(argv[i], "--yes") == 0 || strcmp(argv[i], "-y") == 0) {
            assume_yes = 1;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("\n");
            printf("Usage: stem merge [options] <branch>\n");
            printf("\n");
            printf("Merge a branch into the current branch\n");
            printf("\n");
            printf("Options:\n");
            printf("  --abort           Abort current merge\n");
            printf("  --continue        Continue merge after resolving conflicts\n");
            printf("  --dry-run         Preview merge result only\n");
            printf("  -y, --yes         Skip confirmation prompts\n");
            printf("  --help            Show this help\n");
            printf("\n");
            printf("Examples:\n");
            printf("  stem merge feature-x      # Merge feature-x into current\n");
            printf("  stem merge feature-x --dry-run\n");
            printf("  stem merge --abort        # Cancel merge\n");
            printf("\n");
            printf("Merge Process:\n");
            printf("  1. STEM attempts fast-forward merge (if possible)\n");
            printf("  2. If files conflict, STEM marks them\n");
            printf("  3. Resolve conflicts manually\n");
            printf("  4. Run: stem add <files>\n");
            printf("  5. Run: stem merge --continue\n");
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
    
    /* Handle abort */
    if (abort_merge) {
        /* Check if merge in progress */
        if (!fopen(".stem/MERGE_HEAD", "r")) {
            print_warning("No merge in progress");
            return 0;
        }
        
        remove(".stem/MERGE_HEAD");
        remove(".stem/MERGE_MSG");
        
        printf("\n");
        print_success("Merge aborted");
        printf("\n");
        return 0;
    }
    
    /* Handle continue */
    if (continue_merge) {
        /* Check if merge in progress */
        FILE *merge_head = fopen(".stem/MERGE_HEAD", "r");
        if (!merge_head) {
            print_warning("No merge in progress");
            return 0;
        }
        
        char merge_commit[128];
        fgets(merge_commit, sizeof(merge_commit), merge_head);
        merge_commit[strcspn(merge_commit, "\r\n")] = 0;
        fclose(merge_head);
        
        /* Create merge commit */
        printf("\n");
        print_info("Creating merge commit...");
        printf("\n");
        printf("Merged branch into current branch\n");
        printf("\n");
        printf("Run: stem commit -m \"Merge <branch>\"\n");
        printf("\n");
        
        remove(".stem/MERGE_HEAD");
        remove(".stem/MERGE_MSG");
        
        return 0;
    }
    
    /* Normal merge */
    if (!branch_name) {
        print_error("Please specify a branch to merge");
        printf("\n");
        printf("Usage: stem merge <branch>\n");
        return 1;
    }
    
    /* Get current branch */
    char current_branch[256];
    if (get_current_branch(current_branch, sizeof(current_branch)) != 0) {
        print_error("Could not determine current branch");
        return 1;
    }
    
    /* Check if merging into itself */
    if (strcmp(current_branch, branch_name) == 0) {
        print_error("Cannot merge branch into itself");
        return 1;
    }
    
    /* Get commits */
    char current_commit[128];
    char merge_commit[128];
    
    if (get_branch_commit(current_branch, current_commit, sizeof(current_commit)) != 0) {
        print_error("Could not get current branch commit");
        return 1;
    }
    
    if (get_branch_commit(branch_name, merge_commit, sizeof(merge_commit)) != 0) {
        print_error("Branch '%s' does not exist", branch_name);
        return 1;
    }
    
    /* Check if already up to date */
    if (strcmp(current_commit, merge_commit) == 0) {
        printf("\n");
        print_info("Already up-to-date");
        printf("\n");
        return 0;
    }
    
    /* Attempt fast-forward merge */
    printf("\n");
    printf("================================================================\n");
    printf("  Merging '%s' into '%s'\n", branch_name, current_branch);
    printf("================================================================\n");
    printf("\n");
    print_info("Checking merge strategy...");
    
    /* Check if we can fast-forward (merge_commit is descendant of current) */
    /* For now, check if current_commit is the parent of merge_commit */
    char merge_parent[128];
    char commit_path[512];
    snprintf(commit_path, sizeof(commit_path), ".stem/objects/commits/%s", merge_commit);
    
    FILE *mf = fopen(commit_path, "r");
    int can_ff = 0;
    if (mf) {
        char line[512];
        while (fgets(line, sizeof(line), mf)) {
            if (strncmp(line, "parent ", 7) == 0) {
                line[strcspn(line, "\r\n")] = 0;
                strcpy(merge_parent, line + 7);
                if (strcmp(merge_parent, current_commit) == 0) {
                    can_ff = 1;
                }
                break;
            }
        }
        fclose(mf);
    }
    
    if (can_ff) {
        printf("\n");
        print_info("Fast-forward merge possible");
        if (dry_run) {
            printf("\n");
            print_info("Dry run: branch would fast-forward to %s", merge_commit);
            printf("\n");
            return 0;
        }
        if (!assume_yes && !confirm_merge_action("Apply fast-forward merge now?")) {
            print_warning("Merge cancelled by user");
            printf("\n");
            return 1;
        }
        return fast_forward_merge(branch_name, merge_commit);
    }
    
    /* Need 3-way merge */
    printf("\n");
    print_info("3-way merge required");
    printf("\n");
    
    /* Find merge base */
    char merge_base[128];
    if (find_merge_base(current_commit, merge_commit, merge_base, sizeof(merge_base)) != 0) {
        print_error("Could not find common ancestor");
        return 1;
    }
    
    printf("[*] Merge base: %s\n", merge_base);
    printf("[*] Analyzing file changes...\n");
    printf("\n");
    
    /* Get file lists from both commits */
    char **current_files = NULL, **merge_files = NULL;
    int current_count = 0, merge_count = 0;
    
    get_commit_files(current_commit, &current_files, &current_count);
    get_commit_files(merge_commit, &merge_files, &merge_count);
    
    /* Check for conflicts */
    int conflicts = 0;
    char **conflict_files = NULL;
    int conflict_count = 0;
    
    for (int i = 0; i < current_count; i++) {
        /* Check if file also exists in merge branch */
        for (int j = 0; j < merge_count; j++) {
            if (strcmp(current_files[i], merge_files[j]) == 0) {
                /* Both branches modified this file - check for conflicts */
                if (merge_file_3way(current_files[i], merge_base, current_commit, merge_commit) != 0) {
                    printf("[!] CONFLICT: %s\n", current_files[i]);
                    conflict_files = realloc(conflict_files, sizeof(char *) * (conflict_count + 1));
                    if (conflict_files) {
                        conflict_files[conflict_count++] = strdup(current_files[i]);
                    }
                    conflicts++;
                } else {
                    printf("[OK] %s (no conflict)\n", current_files[i]);
                }
                break;
            }
        }
    }

    if (dry_run) {
        printf("\n");
        print_info("Dry run summary");
        printf("  Current branch: %s (%s)\n", current_branch, current_commit);
        printf("  Merge branch:   %s (%s)\n", branch_name, merge_commit);
        printf("  Potential conflicts: %d\n", conflicts);
    }

    if (!dry_run && !assume_yes) {
        if (!confirm_merge_action("Apply this merge now?")) {
            for (int i = 0; i < conflict_count; i++) free(conflict_files[i]);
            free(conflict_files);
            for (int i = 0; i < current_count; i++) free(current_files[i]);
            for (int i = 0; i < merge_count; i++) free(merge_files[i]);
            free(current_files);
            free(merge_files);
            print_warning("Merge cancelled by user");
            printf("\n");
            return 1;
        }
    }
    
    /* Free file lists */
    for (int i = 0; i < current_count; i++) free(current_files[i]);
    for (int i = 0; i < merge_count; i++) free(merge_files[i]);
    free(current_files);
    free(merge_files);

    if (dry_run) {
        for (int i = 0; i < conflict_count; i++) free(conflict_files[i]);
        free(conflict_files);
        printf("\n");
        return conflicts > 0 ? 1 : 0;
    }

    /* Add conflict markers after explicit confirmation */
    for (int i = 0; i < conflict_count; i++) {
        mark_conflict(conflict_files[i], current_branch, branch_name);
    }

    for (int i = 0; i < conflict_count; i++) free(conflict_files[i]);
    free(conflict_files);
    
    printf("\n");
    
    if (conflicts > 0) {
        /* Create merge state files */
        FILE *merge_head = fopen(".stem/MERGE_HEAD", "w");
        if (merge_head) {
            fprintf(merge_head, "%s\n", merge_commit);
            fclose(merge_head);
        }
        
        FILE *merge_msg = fopen(".stem/MERGE_MSG", "w");
        if (merge_msg) {
            fprintf(merge_msg, "Merge branch '%s' into '%s'\n", branch_name, current_branch);
            fclose(merge_msg);
        }
        
        print_error("Merge conflicts detected!");
        printf("\n");
        printf("Files with conflicts: %d\n", conflicts);
        printf("\n");
        printf("To resolve:\n");
        printf("  1. Edit conflicting files manually\n");
        printf("  2. Run: stem add <files>\n");
        printf("  3. Run: stem commit -m \"Merge %s\"\n", branch_name);
        printf("\n");
        printf("Or abort with: stem merge --abort\n");
        printf("\n");
        return 1;
    }
    
    /* No conflicts - auto-merge */
    printf("\n");
    print_success("Merge successful - no conflicts");
    printf("\n");
    printf("Complete the merge with:\n");
    printf("  stem commit -m \"Merge branch '%s'\"\n", branch_name);
    printf("\n");
    
    /* Create merge state */
    FILE *merge_head = fopen(".stem/MERGE_HEAD", "w");
    if (merge_head) {
        fprintf(merge_head, "%s\n", merge_commit);
        fclose(merge_head);
    }
    
    return 0;
}
