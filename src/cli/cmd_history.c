#include "commands.h"
#include "stem/stem.h"
#include "output.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

/* Read a commit object */
static int read_commit(const char *commit_id, char *message, char *parent, char *author, time_t *timestamp, int *file_count) {
    char commit_path[512];
    snprintf(commit_path, sizeof(commit_path), ".stem/objects/commits/%s", commit_id);
    
    FILE *f = fopen(commit_path, "r");
    if (!f) return -1;
    
    char line[2048];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        
        if (strncmp(line, "parent ", 7) == 0) {
            strcpy(parent, line + 7);
        }
        else if (strncmp(line, "time ", 5) == 0) {
            *timestamp = atol(line + 5);
        }
        else if (strncmp(line, "author ", 7) == 0) {
            strcpy(author, line + 7);
        }
        else if (strncmp(line, "files ", 6) == 0) {
            *file_count = atoi(line + 6);
        }
        else if (strncmp(line, "message ", 8) == 0) {
            strcpy(message, line + 8);
        }
    }
    
    fclose(f);
    return 0;
}

/* Get current HEAD commit */
static int get_head_commit(char *commit_id, size_t size) {
    FILE *head = fopen(".stem/HEAD", "r");
    if (!head) return -1;
    
    char line[512];
    if (fgets(line, sizeof(line), head)) {
        line[strcspn(line, "\r\n")] = 0;
        
        /* If HEAD points to a branch, read branch file */
        if (strncmp(line, "ref: .stem/refs/heads/", 22) == 0) {
            fclose(head);
            FILE *branch = fopen(line + 5, "r");
            if (!branch) return -1;
            
            if (fgets(commit_id, size, branch)) {
                commit_id[strcspn(commit_id, "\r\n")] = 0;
                fclose(branch);
                return 0;
            }
            fclose(branch);
            return -1;
        }
    }
    
    fclose(head);
    return -1;
}

/* History/Log command */
int cmd_history(int argc, char **argv) {
    int max_count = 10;
    int show_graph = 0;
    int oneline = 0;
    const char *author_filter = NULL;
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--all") == 0 || strcmp(argv[i], "-a") == 0) {
            max_count = 100;
        }
        else if (strncmp(argv[i], "-n", 2) == 0) {
            max_count = atoi(argv[i] + 2);
            if (max_count <= 0) max_count = 10;
        }
        else if (strcmp(argv[i], "--graph") == 0) {
            show_graph = 1;
        }
        else if (strcmp(argv[i], "--oneline") == 0) {
            oneline = 1;
        }
        else if (strncmp(argv[i], "--author=", 9) == 0) {
            author_filter = argv[i] + 9;
        }
        else if (strcmp(argv[i], "--author") == 0 && i + 1 < argc) {
            author_filter = argv[++i];
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("\n");
            printf("Usage: stem history [options]\n");
            printf("\n");
            printf("Show commit history\n");
            printf("\n");
            printf("Options:\n");
            printf("  -n<number>        Show last N commits (default: 10)\n");
            printf("  -a, --all         Show all commits\n");
            printf("  --oneline         Compact one-line output\n");
            printf("  --graph           ASCII graph-style output\n");
            printf("  --author <name>   Filter by author\n");
            printf("  --help            Show this help\n");
            printf("\n");
            printf("Examples:\n");
            printf("  stem history              # Show last 10 commits\n");
            printf("  stem history -n20         # Show last 20 commits\n");
            printf("  stem history --all        # Show all commits\n");
            printf("  stem history --oneline    # Compact log\n");
            printf("  stem history --graph      # Graph-style log\n");
            printf("  stem history --author Alice\n");
            printf("\n");
            return 0;
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
    
    /* Get current commit */
    char current_commit[64];
    if (get_head_commit(current_commit, sizeof(current_commit)) != 0) {
        printf("\n");
        print_warning("No commits yet");
        printf("\n");
        printf("Create your first commit:\n");
        printf("  stem add .\n");
        printf("  stem commit -m \"Initial commit\"\n");
        printf("\n");
        return 0;
    }
    
    /* Check if it's the initial empty commit */
    if (strcmp(current_commit, "0000000") == 0) {
        printf("\n");
        print_warning("No commits yet");
        printf("\n");
        return 0;
    }
    
    printf("\n");
    printf("================================================================\n");
    if (oneline) {
        printf("  Commit History (One-line)\n");
    } else if (show_graph) {
        printf("  Commit History (Graph)\n");
    } else {
        printf("  Commit History\n");
    }
    printf("================================================================\n");
    printf("\n");
    
    /* Walk the commit chain */
    char commit_id[64];
    strncpy(commit_id, current_commit, sizeof(commit_id));
    int count = 0;
    
    while (strcmp(commit_id, "0000000") != 0 && count < max_count) {
        char message[512] = {0};
        char parent[64] = {0};
        char author[256] = {0};
        time_t timestamp = 0;
        int files = 0;
        
        if (read_commit(commit_id, message, parent, author, &timestamp, &files) == 0) {
            if (author_filter && author[0] && strstr(author, author_filter) == NULL) {
                strncpy(commit_id, parent, sizeof(commit_id));
                continue;
            }

            if (author_filter && !author[0]) {
                strncpy(commit_id, parent, sizeof(commit_id));
                continue;
            }

            if (oneline) {
                printf("%s %s\n", commit_id, message[0] ? message : "(no message)");
            } else if (show_graph) {
                printf("* %s %s\n", commit_id, message[0] ? message : "(no message)");
                if (author[0]) {
                    printf("| Author: %s\n", author);
                }
                if (timestamp > 0) {
                    struct tm *tm_info = localtime(&timestamp);
                    char time_str[64];
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
                    printf("| Date:   %s\n", time_str);
                }
                printf("| Files:  %d\n", files);
                if (strcmp(parent, "0000000") != 0 && count < max_count - 1) {
                    printf("|\n");
                }
            } else {
                printf("Commit: %s\n", commit_id);
            
                if (author[0]) {
                    printf("Author: %s\n", author);
                }

                if (timestamp > 0) {
                    struct tm *tm_info = localtime(&timestamp);
                    char time_str[64];
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
                    printf("Date:   %s\n", time_str);
                }

                printf("Files:  %d\n", files);
                printf("\n");
                printf("    %s\n", message);
                printf("\n");

                if (count < max_count - 1) {
                    printf("----------------------------------------------------------------\n");
                    printf("\n");
                }
            }
            
            /* Move to parent commit */
            strncpy(commit_id, parent, sizeof(commit_id));
            count++;
        } else {
            /* Couldn't read commit, stop */
            break;
        }
    }
    
    if (count == 0) {
        print_warning("No commits found");
        printf("\n");
    } else {
        printf("================================================================\n");
        printf("\n");
        printf("Showing %d commit(s)\n", count);
        if (author_filter) {
            printf("Filter: author contains '%s'\n", author_filter);
        }
        if (strcmp(commit_id, "0000000") != 0) {
            printf("Use --all to see more\n");
        }
        printf("\n");
    }
    
    return 0;
}

/* Simple line-by-line diff */
static void show_file_diff(const char *file_path, const char *commit_id __attribute__((unused))) {
    /* TODO: Use commit_id to compare against specific commit instead of just last */
    char obj_path[2048];
    snprintf(obj_path, sizeof(obj_path), ".stem/objects/%s", file_path);
    
    /* Read both files */
    FILE *current = fopen(file_path, "r");
    FILE *committed = fopen(obj_path, "r");
    
    if (!current && !committed) {
        return;
    }
    
    printf("\n");
    printf("--- %s (committed)\n", file_path);
    printf("+++ %s (working)\n", file_path);
    printf("\n");
    
    if (!committed && current) {
        printf("[New file]\n");
        fclose(current);
        return;
    }
    
    if (committed && !current) {
        printf("[Deleted file]\n");
        fclose(committed);
        return;
    }
    
    /* Compare line by line */
    char line1[2048], line2[2048];
    int line_num = 0;
    int changes = 0;
    
    while (1) {
        char *r1 = fgets(line1, sizeof(line1), committed);
        char *r2 = fgets(line2, sizeof(line2), current);
        
        if (!r1 && !r2) break;
        
        line_num++;
        
        if (!r1) {
            printf("+ %4d: %s", line_num, line2);
            changes++;
        }
        else if (!r2) {
            printf("- %4d: %s", line_num, line1);
            changes++;
        }
        else if (strcmp(line1, line2) != 0) {
            printf("- %4d: %s", line_num, line1);
            printf("+ %4d: %s", line_num, line2);
            changes++;
        }
    }
    
    if (changes == 0) {
        printf("[No changes]\n");
    }
    
    fclose(current);
    fclose(committed);
}

/* Diff command */
int cmd_diff(int argc, char **argv) {
    const char *file_path = NULL;
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("\n");
            printf("Usage: stem diff [file] [commit]\n");
            printf("\n");
            printf("Show differences between working directory and last commit\n");
            printf("\n");
            printf("Options:\n");
            printf("  [file]            Diff specific file\n");
            printf("  [commit]          Diff against specific commit\n");
            printf("  --help            Show this help\n");
            printf("\n");
            printf("Examples:\n");
            printf("  stem diff                # Show all changes\n");
            printf("  stem diff file.txt       # Show changes in file.txt\n");
            printf("\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            file_path = argv[i];
            /* TODO: Support commit_id as second argument for comparing specific commits */
        }
    }
    
    printf("\n");
    printf("================================================================\n");
    printf("  Changes Since Last Commit\n");
    printf("================================================================\n");
    
    /* Check if repository exists */
    FILE *head = fopen(".stem/HEAD", "r");
    if (!head) {
        printf("\n");
        print_error("Not a STEM repository");
        return 1;
    }
    fclose(head);
    
    /* Check if there's a commit */
    char current_commit[64];
    if (get_head_commit(current_commit, sizeof(current_commit)) != 0 ||
        strcmp(current_commit, "0000000") == 0) {
        printf("\n");
        print_warning("No commits yet - nothing to diff against");
        printf("\n");
        return 0;
    }
    
    if (file_path) {
        /* Diff specific file */
        show_file_diff(file_path, current_commit);
    } else {
        /* Diff all files from index */
        FILE *index = fopen(".stem/index", "r");
        if (!index) {
            printf("\n");
            print_warning("No files tracked");
            printf("\n");
            return 0;
        }
        
        char line[2048];
        int file_count = 0;
        while (fgets(line, sizeof(line), index)) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) > 0) {
                show_file_diff(line, current_commit);
                file_count++;
            }
        }
        fclose(index);
        
        if (file_count == 0) {
            printf("\n");
            print_warning("No files to diff");
            printf("\n");
        }
    }
    
    printf("\n");
    printf("================================================================\n");
    printf("\n");
    
    return 0;
}

/* Copy file helper */
static int restore_file(const char *src, const char *dst) {
    FILE *source = fopen(src, "rb");
    if (!source) return -1;
    
    FILE *dest = fopen(dst, "wb");
    if (!dest) {
        fclose(source);
        return -1;
    }
    
    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytes, dest);
    }
    
    fclose(source);
    fclose(dest);
    return 0;
}

static int confirm_action(const char *prompt) {
    char answer[16];
    printf("%s [y/N]: ", prompt);
    if (!fgets(answer, sizeof(answer), stdin)) {
        return 0;
    }
    return (answer[0] == 'y' || answer[0] == 'Y');
}

/* Restore command */
int cmd_restore(int argc, char **argv) {
    const char *file_path = NULL;
    int restore_all = 0;
    int dry_run = 0;
    int assume_yes = 0;
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--all") == 0 || strcmp(argv[i], "-a") == 0) {
            restore_all = 1;
        }
        else if (strcmp(argv[i], "--dry-run") == 0) {
            dry_run = 1;
        }
        else if (strcmp(argv[i], "--yes") == 0 || strcmp(argv[i], "-y") == 0) {
            assume_yes = 1;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("\n");
            printf("Usage: stem restore [options] [file]\n");
            printf("\n");
            printf("Restore files from last commit\n");
            printf("\n");
            printf("Options:\n");
            printf("  [file]            Restore specific file\n");
            printf("  -a, --all         Restore all tracked files\n");
            printf("  --dry-run         Preview files without restoring\n");
            printf("  -y, --yes         Skip confirmation prompt\n");
            printf("  --help            Show this help\n");
            printf("\n");
            printf("Examples:\n");
            printf("  stem restore file.txt    # Restore file.txt\n");
            printf("  stem restore --all       # Restore all files\n");
            printf("  stem restore --all --dry-run\n");
            printf("\n");
            printf("WARNING: This will overwrite your current changes!\n");
            printf("\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            file_path = argv[i];
        }
    }
    
    if (!file_path && !restore_all) {
        print_error("Please specify a file or use --all");
        printf("\n");
        printf("Usage: stem restore <file>\n");
        printf("       stem restore --all\n");
        return 1;
    }
    
    /* Check repository */
    FILE *head = fopen(".stem/HEAD", "r");
    if (!head) {
        print_error("Not a STEM repository");
        return 1;
    }
    fclose(head);
    
    /* Check if there's a commit */
    char current_commit[64];
    if (get_head_commit(current_commit, sizeof(current_commit)) != 0 ||
        strcmp(current_commit, "0000000") == 0) {
        print_error("No commits yet - nothing to restore from");
        return 1;
    }
    
    printf("\n");
    printf("================================================================\n");
    printf("  Restoring Files\n");
    printf("================================================================\n");
    printf("\n");
    
    int restored = 0;
    int preview_count = 0;
    
    if (restore_all) {
        /* Restore all files from index */
        FILE *index = fopen(".stem/index", "r");
        if (!index) {
            print_error("No files tracked");
            return 1;
        }

        if (!dry_run && !assume_yes) {
            if (!confirm_action("Restore ALL tracked files and overwrite local changes?")) {
                fclose(index);
                print_warning("Restore cancelled by user");
                printf("\n");
                return 1;
            }
        }
        
        char line[2048];
        while (fgets(line, sizeof(line), index)) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) > 0) {
                char obj_path[2048];
                snprintf(obj_path, sizeof(obj_path), ".stem/objects/%s", line);

                if (dry_run) {
                    printf("[*] Would restore: %s\n", line);
                    preview_count++;
                    continue;
                }

                if (restore_file(obj_path, line) == 0) {
                    printf("[*] Restored: %s\n", line);
                    restored++;
                } else {
                    print_warning("Could not restore: %s", line);
                }
            }
        }
        fclose(index);
    } else {
        /* Restore specific file */
        char obj_path[2048];
        snprintf(obj_path, sizeof(obj_path), ".stem/objects/%s", file_path);

        if (!dry_run && !assume_yes) {
            char prompt[2300];
            snprintf(prompt, sizeof(prompt), "Restore '%s' and overwrite local changes?", file_path);
            if (!confirm_action(prompt)) {
                print_warning("Restore cancelled by user");
                printf("\n");
                return 1;
            }
        }
        
        if (dry_run) {
            printf("[*] Would restore: %s\n", file_path);
            preview_count = 1;
        }
        else if (restore_file(obj_path, file_path) == 0) {
            printf("[*] Restored: %s\n", file_path);
            restored = 1;
        } else {
            print_error("Could not restore: %s", file_path);
            printf("\n");
            printf("File may not be tracked or committed.\n");
            return 1;
        }
    }
    
    if (dry_run) {
        printf("\n");
        print_info("Dry run complete");
        printf("\n");
        printf("  Files to restore: %d\n", preview_count);
        return 0;
    }

    printf("\n");
    if (restored > 0) {
        print_success("Restored %d file(s)", restored);
    } else {
        print_warning("No files restored");
    }
    printf("\n");
    
    return 0;
}

/* Verify command */
int cmd_verify(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("\n");
    printf("================================================================\n");
    printf("  Repository Verification\n");
    printf("================================================================\n");
    printf("\n");
    
    int errors = 0;
    
    /* Check .stem directory */
    printf("[*] Checking repository structure...\n");
    FILE *head = fopen(".stem/HEAD", "r");
    if (head) {
        fclose(head);
        print_success("  HEAD file exists");
    } else {
        print_error("  HEAD file missing");
        errors++;
    }
    
    /* Check refs directory */
    #ifdef _WIN32
    DWORD attr = GetFileAttributesA(".stem/refs");
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        print_success("  refs directory exists");
    } else {
        print_warning("  refs directory missing");
    }
    #else
    struct stat st;
    if (stat(".stem/refs", &st) == 0 && S_ISDIR(st.st_mode)) {
        print_success("  refs directory exists");
    } else {
        print_warning("  refs directory missing");
    }
    #endif
    
    /* Check objects directory */
    #ifdef _WIN32
    attr = GetFileAttributesA(".stem/objects");
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        print_success("  objects directory exists");
    } else {
        print_warning("  objects directory missing");
    }
    #else
    if (stat(".stem/objects", &st) == 0 && S_ISDIR(st.st_mode)) {
        print_success("  objects directory exists");
    } else {
        print_warning("  objects directory missing");
    }
    #endif
    
    printf("\n");
    
    if (errors == 0) {
        print_success("Repository structure is healthy");
    } else {
        print_error("Found %d error(s)", errors);
    }
    
    printf("\n");
    return errors > 0 ? 1 : 0;
}
