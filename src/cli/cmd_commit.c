#include "commands.h"
#include "stem/stem.h"
#include "output.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

/* Fast file copy function with larger buffer */
static int copy_file(const char *src, const char *dst) {
    FILE *source = fopen(src, "rb");
    if (!source) return -1;
    
    FILE *dest = fopen(dst, "wb");
    if (!dest) {
        fclose(source);
        return -1;
    }
    
    /* Use 64KB buffer for faster copying */
    char *buffer = (char *)malloc(65536);
    if (!buffer) {
        fclose(source);
        fclose(dest);
        return -1;
    }
    
    size_t bytes;
    while ((bytes = fread(buffer, 1, 65536, source)) > 0) {
        if (fwrite(buffer, 1, bytes, dest) != bytes) {
            free(buffer);
            fclose(source);
            fclose(dest);
            return -1;
        }
    }
    
    free(buffer);
    fclose(source);
    fclose(dest);
    return 0;
}

/* Create directory recursively */
static int mkdir_p(const char *path) {
    char tmp[1024];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
    return 0;
}

static void trim_whitespace(char *s) {
    char *start = s;
    char *end;

    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    *end = '\0';
}

static int get_config_value(const char *key, char *value, size_t value_size) {
    const char *paths[] = {".stem/config", NULL};
    char global_path[512];
    const char *home = getenv("HOME");
    if (!home) {
        home = getenv("USERPROFILE");
    }

    if (home) {
        snprintf(global_path, sizeof(global_path), "%s/.stemconfig", home);
        paths[1] = global_path;
    }

    for (int p = 0; p < 2; p++) {
        if (!paths[p]) continue;

        FILE *f = fopen(paths[p], "r");
        if (!f) continue;

        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\r\n")] = 0;
            char *eq = strchr(line, '=');
            if (!eq) continue;

            *eq = '\0';
            char *line_key = line;
            char *line_val = eq + 1;
            trim_whitespace(line_key);
            trim_whitespace(line_val);

            if (strcmp(line_key, key) == 0) {
                strncpy(value, line_val, value_size - 1);
                value[value_size - 1] = '\0';
                fclose(f);
                return 0;
            }
        }

        fclose(f);
    }

    return -1;
}

static void get_commit_author(char *author, size_t size) {
    char name[256] = {0};
    char email[256] = {0};

    get_config_value("user.name", name, sizeof(name));
    get_config_value("user.email", email, sizeof(email));

    if (name[0] && email[0]) {
        snprintf(author, size, "%s <%s>", name, email);
    } else if (name[0]) {
        snprintf(author, size, "%s", name);
    } else {
        snprintf(author, size, "unknown");
    }
}

static int get_commit_message_from_editor(char *message, size_t size, int amend_mode) {
    const char *editor = getenv("STEM_EDITOR");
    if (!editor || !editor[0]) editor = getenv("EDITOR");

    char configured_editor[256] = {0};
    if ((!editor || !editor[0]) && get_config_value("core.editor", configured_editor, sizeof(configured_editor)) == 0) {
        editor = configured_editor;
    }

    if (!editor || !editor[0]) {
#ifdef _WIN32
        editor = "notepad";
#else
        editor = "vi";
#endif
    }

    FILE *tmpl = fopen(".stem/COMMIT_EDITMSG", "w");
    if (!tmpl) {
        return -1;
    }

    fprintf(tmpl, "# Write your commit message above.\n");
    fprintf(tmpl, "# Lines starting with # are ignored.\n");
    fprintf(tmpl, "# Empty message aborts the commit.\n");
    if (amend_mode) {
        fprintf(tmpl, "# Amending previous commit.\n");
    }
    fclose(tmpl);

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "\"%s\" \".stem/COMMIT_EDITMSG\"", editor);

    if (system(cmd) != 0) {
        return -1;
    }

    FILE *msg = fopen(".stem/COMMIT_EDITMSG", "r");
    if (!msg) {
        return -1;
    }

    char line[1024];
    size_t used = 0;
    message[0] = '\0';

    while (fgets(line, sizeof(line), msg)) {
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '#') continue;
        trim_whitespace(line);
        if (!line[0]) continue;

        size_t len = strlen(line);
        if (used > 0 && used + 1 < size) {
            message[used++] = ' ';
            message[used] = '\0';
        }

        if (used + len >= size) {
            len = size - used - 1;
        }

        if (len > 0) {
            memcpy(message + used, line, len);
            used += len;
            message[used] = '\0';
        }
    }

    fclose(msg);
    return (message[0] != '\0') ? 0 : -1;
}

int cmd_commit(int argc, char **argv) {
    const char *message = NULL;
    char editor_message[1024] = {0};
    char author[512] = {0};
    int amend = 0;
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            message = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "--amend") == 0) {
            amend = 1;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: stem commit [options]\n\n");
            printf("Create a new commit\n\n");
            printf("Options:\n");
            printf("  -m <message>  Commit message\n");
            printf("  --amend       Amend the last commit\n");
            printf("  --help        Show this help message\n\n");
            printf("Examples:\n");
            printf("  stem commit -m \"Add feature\"\n");
            printf("  stem commit                  # Opens your editor\n");
            printf("  stem commit --amend -m \"Fix typo in last commit\"\n");
            return 0;
        }
    }
    
    if (!message) {
        print_info("Opening commit message editor...");
        if (get_commit_message_from_editor(editor_message, sizeof(editor_message), amend) != 0) {
            print_error("Empty or invalid commit message");
            printf("\nTip: use -m \"message\" or configure core.editor\n");
            return 1;
        }
        message = editor_message;
    }

    get_commit_author(author, sizeof(author));
    
    if (amend) {
        /* Get current HEAD commit */
        FILE *head = fopen(".stem/HEAD", "r");
        if (!head) {
            print_error("Not a STEM repository");
            return 1;
        }
        
        char head_line[512];
        char current_commit[64] = "0000000";
        
        if (fgets(head_line, sizeof(head_line), head)) {
            head_line[strcspn(head_line, "\r\n")] = 0;
            
            if (strncmp(head_line, "ref: .stem/refs/heads/", 22) == 0) {
                FILE *branch = fopen(head_line + 5, "r");
                if (branch) {
                    if (fgets(current_commit, sizeof(current_commit), branch)) {
                        current_commit[strcspn(current_commit, "\r\n")] = 0;
                    }
                    fclose(branch);
                }
            }
        }
        fclose(head);
        
        if (strcmp(current_commit, "0000000") == 0) {
            print_error("No commit to amend");
            return 1;
        }
        
        printf("\n");
        printf("================================================================\n");
        printf("  Amending Commit %s\n", current_commit);
        printf("================================================================\n");
        printf("\n");
        print_info("Updating commit message...");
        
        /* Update commit object */
        char commit_path[512];
        snprintf(commit_path, sizeof(commit_path), ".stem/objects/commits/%s", current_commit);
        
        /* Read existing commit */
        FILE *commit_file = fopen(commit_path, "r");
        if (!commit_file) {
            print_error("Could not read commit");
            return 1;
        }
        
        char **lines = NULL;
        int line_count = 0;
        char line[2048];
        
        while (fgets(line, sizeof(line), commit_file)) {
            line[strcspn(line, "\r\n")] = 0;
            
            /* Replace message line */
            if (strncmp(line, "message ", 8) == 0) {
                snprintf(line, sizeof(line), "message %s", message);
            }
            
            lines = realloc(lines, sizeof(char*) * (line_count + 1));
            lines[line_count] = strdup(line);
            line_count++;
        }
        fclose(commit_file);
        
        /* Write back */
        commit_file = fopen(commit_path, "w");
        if (!commit_file) {
            print_error("Could not update commit");
            return 1;
        }
        
        for (int i = 0; i < line_count; i++) {
            fprintf(commit_file, "%s\n", lines[i]);
            free(lines[i]);
        }
        free(lines);
        fclose(commit_file);
        
        printf("\n");
        print_success("Commit amended!");
        printf("\n");
        printf("  Commit:  %s\n", current_commit);
        printf("  Message: %s\n", message);
        printf("\n");
        
        return 0;
    }
    
    /* Normal commit (rest of existing code) */
    printf("================================================================\n");
    printf("  Creating Commit\n");
    printf("================================================================\n");
    printf("\n");
    printf("  Message: %s\n", message);
    printf("  Author:  %s\n", author);
    
    /* Create objects directory if it doesn't exist */
    mkdir_p(".stem/objects");
    
    /* Read index file to get staged files */
    FILE *index_file = fopen(".stem/index", "r");
    if (!index_file) {
        printf("\n");
        print_error("No files staged for commit");
        printf("\n");
        printf("Use 'stem add <file>' to stage files first.\n");
        printf("Example: stem add .\n");
        printf("\n");
        return 1;
    }
    
    /* Count files and copy them to objects */
    int file_count = 0;
    char line[2048];
    
    /* First pass: count total files */
    int num_files = 0;
    while (fgets(line, sizeof(line), index_file)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) > 0) {
            num_files++;
        }
    }
    
    if (num_files == 0) {
        printf("\n");
        print_error("No files staged for commit");
        printf("\n");
        fclose(index_file);
        return 1;
    }
    
    /* Allocate dynamic array for all files */
    char **files = (char **)malloc(num_files * sizeof(char *));
    if (!files) {
        print_error("Out of memory");
        fclose(index_file);
        return 1;
    }
    
    /* Second pass: read all file paths */
    rewind(index_file);
    int file_index = 0;
    while (fgets(line, sizeof(line), index_file) && file_index < num_files) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) > 0) {
            files[file_index] = strdup(line);
            file_index++;
        }
    }
    fclose(index_file);
    
    printf("  Files:   %d staged\n", num_files);
    printf("\n");
    printf("[*] Storing files in repository...\n");
    printf("\n");
    
    /* Copy each file to objects directory */
    int last_percent = -1;
    for (int i = 0; i < num_files; i++) {
        char *file_path = files[i];
        
        int percent = (int)(((i + 1) * 100) / num_files);
        if (percent != last_percent && (percent % 10 == 0 || i == num_files - 1)) {
            printf("  Progress: %d%% (%d/%d files)\n", percent, i + 1, num_files);
            last_percent = percent;
        }
        
        /* Create object path preserving directory structure */
        /* Convert absolute path to relative path from current directory */
        char relative_path[2048];
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        
        /* If file path starts with cwd, make it relative */
        if (strncmp(file_path, cwd, strlen(cwd)) == 0) {
            const char *rel = file_path + strlen(cwd);
            while (*rel == '\\' || *rel == '/') rel++;
            strncpy(relative_path, rel, sizeof(relative_path) - 1);
        } else {
            strncpy(relative_path, file_path, sizeof(relative_path) - 1);
        }
        
        /* Create full object path: .stem/objects/{relative_path} */
        char obj_path[2048];
        snprintf(obj_path, sizeof(obj_path), ".stem/objects/%s", relative_path);
        
        /* Create parent directories for the object */
        char *last_sep = strrchr(obj_path, '/');
        if (!last_sep) last_sep = strrchr(obj_path, '\\');
        if (last_sep) {
            char dir_path[2048];
            size_t dir_len = last_sep - obj_path;
            strncpy(dir_path, obj_path, dir_len);
            dir_path[dir_len] = '\0';
            mkdir_p(dir_path);
        }
        
        /* Copy file */
        if (copy_file(file_path, obj_path) == 0) {
            file_count++;
        }
        
        free(file_path);
    }
    
    /* Generate a commit ID based on timestamp and file count */
    time_t now = time(NULL);
    char commit_id[8];
    snprintf(commit_id, sizeof(commit_id), "%07lx", (long)(now + file_count) & 0xFFFFFFF);
    
    /* Get parent commit (current HEAD) */
    char parent_id[64] = "0000000";
    FILE *head_file = fopen(".stem/HEAD", "r");
    if (head_file) {
        char head_line[512];
        if (fgets(head_line, sizeof(head_line), head_file)) {
            head_line[strcspn(head_line, "\r\n")] = 0;
            
            /* If HEAD points to a branch, read the branch file */
            if (strncmp(head_line, "ref: .stem/refs/heads/", 22) == 0) {
                FILE *branch_file = fopen(head_line + 5, "r");
                if (branch_file) {
                    if (fgets(parent_id, sizeof(parent_id), branch_file)) {
                        parent_id[strcspn(parent_id, "\r\n")] = 0;
                    }
                    fclose(branch_file);
                }
            }
        }
        fclose(head_file);
    }
    
    /* Create commit object file */
    char commit_obj_path[256];
    snprintf(commit_obj_path, sizeof(commit_obj_path), ".stem/objects/commits/%s", commit_id);
    mkdir_p(".stem/objects/commits");
    
    FILE *commit_file = fopen(commit_obj_path, "w");
    if (commit_file) {
        fprintf(commit_file, "commit %s\n", commit_id);
        fprintf(commit_file, "parent %s\n", parent_id);
        fprintf(commit_file, "time %ld\n", (long)now);
        fprintf(commit_file, "author %s\n", author);
        fprintf(commit_file, "files %d\n", file_count);
        fprintf(commit_file, "message %s\n", message);
        fprintf(commit_file, "\n");
        
        /* Write file list */
        fprintf(commit_file, "--- files ---\n");
        FILE *idx = fopen(".stem/index", "r");
        if (idx) {
            char fline[2048];
            while (fgets(fline, sizeof(fline), idx)) {
                fprintf(commit_file, "%s", fline);
            }
            fclose(idx);
        }
        
        fclose(commit_file);
    }
    
    /* Update current branch to point to new commit */
    head_file = fopen(".stem/HEAD", "r");
    if (head_file) {
        char head_line[512];
        if (fgets(head_line, sizeof(head_line), head_file)) {
            head_line[strcspn(head_line, "\r\n")] = 0;
            
            /* If HEAD points to a branch, update it */
            if (strncmp(head_line, "ref: .stem/refs/heads/", 22) == 0) {
                FILE *branch_file = fopen(head_line + 5, "w");
                if (branch_file) {
                    fprintf(branch_file, "%s\n", commit_id);
                    fclose(branch_file);
                }
            }
        }
        fclose(head_file);
    }
    
    /* Keep legacy COMMIT_MSG for compatibility */
    FILE *legacy_commit = fopen(".stem/COMMIT_MSG", "w");
    if (legacy_commit) {
        fprintf(legacy_commit, "%s\n", message);
        fprintf(legacy_commit, "Files: %d\n", file_count);
        fprintf(legacy_commit, "Time: %ld\n", (long)now);
        fclose(legacy_commit);
    }
    
    /* Free the files array */
    free(files);
    
    printf("\n");
    printf("================================================================\n");
    printf("  Commit Successful\n");
    printf("================================================================\n");
    printf("\n");
    print_success("Commit ID: %s", commit_id);
    printf("\n");
    printf("  Stored:  %d file(s)\n", file_count);
    printf("  Message: %s\n", message);
    printf("\n");
    printf("Next step: stem push\n");
    printf("\n");
    
    return 0;
}
