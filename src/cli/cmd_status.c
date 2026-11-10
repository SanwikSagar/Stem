#include "commands.h"
#include "stem/stem.h"
#include "output.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir
static int stem_path_exists(const char *path) {
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES);
}
#else
#include <sys/stat.h>
#include <unistd.h>
static int stem_path_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0);
}
#endif

int cmd_status(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("\n");
    printf("================================================================\n");
    printf("  Repository Status\n");
    printf("================================================================\n");
    printf("\n");
    
    /* Check if repository exists */
    if (!stem_path_exists(".stem")) {
        print_error("Not a STEM repository");
        printf("\n");
        printf("Initialize with: stem init\n");
        return 1;
    }
    
    /* Get current branch */
    char branch[256] = "unknown";
    FILE *head = fopen(".stem/HEAD", "r");
    if (head) {
        char line[512];
        if (fgets(line, sizeof(line), head)) {
            line[strcspn(line, "\r\n")] = 0;
            if (strncmp(line, "ref: .stem/refs/heads/", 22) == 0) {
                size_t len = strlen(line + 22);
                if (len > sizeof(branch) - 1) len = sizeof(branch) - 1;
                memcpy(branch, line + 22, len);
                branch[len] = '\0';
            } else if (strncmp(line, "ref: refs/heads/", 16) == 0) {
                size_t len = strlen(line + 16);
                if (len > sizeof(branch) - 1) len = sizeof(branch) - 1;
                memcpy(branch, line + 16, len);
                branch[len] = '\0';
            }
        }
        fclose(head);
    }
    
    printf("On branch: ");
    print_success("%s", branch);
    printf("\n");
    
    /* Get commit count */
    char commit_id[64] = "0000000";
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".stem/refs/heads/%s", branch);
    
    FILE *branch_file = fopen(branch_path, "r");
    if (branch_file) {
        if (fgets(commit_id, sizeof(commit_id), branch_file)) {
            commit_id[strcspn(commit_id, "\r\n")] = 0;
        }
        fclose(branch_file);
    }
    
    if (strcmp(commit_id, "0000000") == 0) {
        printf("No commits yet\n");
        printf("\n");
        print_info("Create your first commit:");
        printf("  1. stem add .\n");
        printf("  2. stem commit -m \"Initial commit\"\n");
        printf("  3. stem push\n");
    } else {
        printf("Latest commit: %s\n", commit_id);
        
        /* Read commit info */
        char commit_path[512];
        snprintf(commit_path, sizeof(commit_path), ".stem/objects/commits/%s", commit_id);
        FILE *commit_file = fopen(commit_path, "r");
        
        if (commit_file) {
            char line[2048];
            while (fgets(line, sizeof(line), commit_file)) {
                line[strcspn(line, "\r\n")] = 0;
                if (strncmp(line, "message ", 8) == 0) {
                    printf("Message: %s\n", line + 8);
                }
                else if (strncmp(line, "files ", 6) == 0) {
                    printf("Files: %s\n", line + 6);
                }
                else if (strncmp(line, "time ", 5) == 0) {
                    time_t t = atol(line + 5);
                    struct tm *tm_info = localtime(&t);
                    char time_str[64];
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
                    printf("Date: %s\n", time_str);
                }
            }
            fclose(commit_file);
        }
    }
    
    printf("\n");
    
    /* Check for staged files */
    FILE *index = fopen(".stem/index", "r");
    if (index) {
        int staged_count = 0;
        char line[2048];
        while (fgets(line, sizeof(line), index)) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) > 0) {
                staged_count++;
            }
        }
        fclose(index);
        
        if (staged_count > 0) {
            printf("Staged files: %d\n", staged_count);
            print_info("Ready to commit:");
            printf("  stem commit -m \"Your message\"\n");
            printf("\n");
        }
    }
    
    /* Check for remote */
    FILE *config = fopen(".stem/config", "r");
    if (config) {
        char line[512];
        bool has_remote = false;
        while (fgets(line, sizeof(line), config)) {
            if (strstr(line, "[remote")) {
                has_remote = true;
                break;
            }
        }
        fclose(config);
        
        if (has_remote) {
            printf("Remote: configured\n");
            print_info("Push to backup:");
            printf("  stem push\n");
        } else {
            print_warning("No remote configured");
            printf("\n");
            printf("Add Google Drive backup:\n");
            printf("  stem remote add cloud <gdrive-url>\n");
        }
    }
    
    printf("\n");
    printf("================================================================\n");
    printf("\n");
    
    return 0;
}

int cmd_clone(int argc, char **argv) {
    const char *remote_url = NULL;
    const char *dest_path = ".";
    
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: stem clone <google-drive-url> [destination]\n\n");
            printf("Clone a repository from Google Drive\n\n");
            printf("Arguments:\n");
            printf("  <url>          Google Drive folder URL\n");
            printf("  [destination]  Local directory (default: current)\n\n");
            printf("Examples:\n");
            printf("  stem clone https://drive.google.com/drive/folders/ABC123\n");
            printf("  stem clone https://drive.google.com/drive/folders/ABC123 my-project\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            if (!remote_url) {
                remote_url = argv[i];
            } else {
                dest_path = argv[i];
            }
        }
    }
    
    if (!remote_url) {
        print_error("Google Drive URL required");
        printf("\nUsage: stem clone <google-drive-url> [destination]\n");
        return 1;
    }
    
    printf("\n");
    printf("================================================================\n");
    printf("  Cloning from Google Drive\n");
    printf("================================================================\n");
    printf("\n");
    
    /* Extract folder ID from URL */
    char folder_id[256] = {0};
    const char *folders_prefix = "/folders/";
    const char *folders_pos = strstr(remote_url, folders_prefix);
    
    if (folders_pos) {
        const char *id_start = folders_pos + strlen(folders_prefix);
        const char *id_end = strchr(id_start, '?');
        if (!id_end) id_end = strchr(id_start, '/');
        if (!id_end) id_end = id_start + strlen(id_start);
        
        size_t id_len = id_end - id_start;
        if (id_len > 0 && id_len < sizeof(folder_id)) {
            strncpy(folder_id, id_start, id_len);
            folder_id[id_len] = '\0';
        }
    }
    
    if (strlen(folder_id) == 0) {
        print_error("Could not extract folder ID from URL");
        printf("\nExpected format: https://drive.google.com/drive/folders/FOLDER-ID\n");
        return 1;
    }
    
    printf("Folder ID: %s\n", folder_id);
    printf("Destination: %s\n", dest_path);
    printf("\n");
    
    /* Create destination directory */
    if (strcmp(dest_path, ".") != 0) {
        #ifdef _WIN32
        if (_mkdir(dest_path) != 0 && errno != EEXIST) {
        #else
        if (mkdir(dest_path, 0755) != 0 && errno != EEXIST) {
        #endif
            print_error("Could not create destination directory");
            return 1;
        }
        
        /* Change to destination directory */
        if (chdir(dest_path) != 0) {
            print_error("Could not enter destination directory");
            return 1;
        }
    }
    
    /* Initialize repository */
    print_info("Initializing repository...");
    if (cmd_init(1, argv) != 0) {
        print_error("Failed to initialize repository");
        return 1;
    }
    
    /* Add remote */
    print_info("Adding remote...");
    char *remote_args[] = {"remote", "add", "cloud", (char*)remote_url};
    if (cmd_remote(4, remote_args) != 0) {
        print_error("Failed to add remote");
        return 1;
    }
    
    /* Pull from remote */
    print_info("Downloading from Google Drive...");
    printf("\n");
    
    char *pull_args[] = {"pull", "cloud"};
    if (cmd_pull(2, pull_args) != 0) {
        print_error("Failed to pull from remote");
        return 1;
    }
    
    printf("\n");
    print_success("Clone complete!");
    printf("\n");
    printf("Repository cloned to: %s\n", dest_path);
    printf("\n");
    
    return 0;
}

int cmd_push(int argc, char **argv) {
    const char *remote_name = "cloud";
    bool dry_run = false;
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dry-run") == 0) {
            dry_run = true;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: stem push [remote] [options]\n\n");
            printf("Push commits to cloud storage\n\n");
            printf("Arguments:\n");
            printf("  remote         Remote name (default: cloud)\n\n");
            printf("Options:\n");
            printf("  --dry-run      Show what would be pushed\n");
            printf("  --help         Show this help message\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            remote_name = argv[i];
        }
    }
    
    printf("\n");
    print_info("Pushing to remote '%s'...", remote_name);
    
    /* Check if .stem directory exists */
    if (!stem_path_exists(".stem")) {
        print_error("Not a STEM repository");
        printf("\nRun 'stem init' first to initialize a repository.\n");
        return 1;
    }
    
    /* Load remote configuration from .stem/config */
    FILE *config = fopen(".stem/config", "r");
    if (!config) {
        print_error("No remote configuration found");
        printf("\nAdd a remote with:\n");
        printf("  stem remote add %s <google-drive-url>\n", remote_name);
        return 1;
    }
    
    char folder_id[256] = {0};
    char provider[64] = {0};
    int keep_backups = 0;
    char line[512];
    bool in_remote_section = false;
    
    while (fgets(line, sizeof(line), config)) {
        /* Remove newline */
        line[strcspn(line, "\r\n")] = 0;
        
        /* Check for remote section */
        if (strstr(line, "[remote \"cloud\"]")) {
            in_remote_section = true;
            continue;
        }
        
        if (in_remote_section) {
            /* Parse folderid */
            if (strstr(line, "folderid")) {
                char *eq = strchr(line, '=');
                if (eq) {
                    eq++;
                    while (*eq == ' ') eq++;
                    strncpy(folder_id, eq, sizeof(folder_id) - 1);
                }
            }
            /* Parse provider */
            if (strstr(line, "provider")) {
                char *eq = strchr(line, '=');
                if (eq) {
                    eq++;
                    while (*eq == ' ') eq++;
                    strncpy(provider, eq, sizeof(provider) - 1);
                }
            }

            if (strstr(line, "keep_backups")) {
                char *eq = strchr(line, '=');
                if (eq) {
                    eq++;
                    while (*eq == ' ') eq++;
                    keep_backups = atoi(eq);
                }
            }
        }

        if (strstr(line, "cloud.keep_backups")) {
            char *eq = strchr(line, '=');
            if (eq) {
                eq++;
                while (*eq == ' ') eq++;
                keep_backups = atoi(eq);
            }
        }
    }
    fclose(config);
    
    if (strlen(folder_id) == 0) {
        print_error("Google Drive folder ID not found in config");
        printf("\nReinitialize with:\n");
        printf("  stem init --cloud <google-drive-url>\n");
        return 1;
    }
    
    printf("  Remote: %s (%s)\n", remote_name, provider);
    printf("  Folder: %s\n", folder_id);
    if (keep_backups > 0) {
        printf("  Retention: keep last %d backup(s)\n", keep_backups);
    }
    
    if (dry_run) {
        printf("\n");
        print_info("Dry run - no data transferred");
        printf("  Would push:\n");
        printf("    - Commit objects from .stem/objects\n");
        printf("    - References from .stem/refs\n");
        printf("    - Configuration files\n");
        return 0;
    }
    
    printf("\n");
    print_info("Preparing upload...");
    
    /* Get current working directory - this is what we'll upload */
    char cwd[512];
    if (!getcwd(cwd, sizeof(cwd))) {
        print_error("Failed to get current directory");
        return 1;
    }
    
    printf("  Source: %s\n", cwd);
    printf("  Note: Uploading your project files (respecting .stemignore)\n");
    
    /* Check for PowerShell upload script */
    char script_path[512];
    #ifdef _WIN32
    /* Get STEM installation directory from executable path */
    char exe_path[512];
    GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
    char *last_slash = strrchr(exe_path, '\\');
    if (last_slash) *last_slash = '\0';
    /* Go up one directory from bin\ */
    last_slash = strrchr(exe_path, '\\');
    if (last_slash) *last_slash = '\0';
    snprintf(script_path, sizeof(script_path), "%s\\gdrive_upload.ps1", exe_path);
    #else
    /* Linux: assume script is in /usr/local/share/stem/ or similar */
    strncpy(script_path, "/usr/local/share/stem/gdrive_upload.ps1", sizeof(script_path) - 1);
    #endif
    
    if (!stem_path_exists(script_path)) {
        print_error("Google Drive upload script not found");
        printf("\nExpected location: %s\n", script_path);
        printf("\nPlease ensure STEM is properly installed.\n");
        return 1;
    }
    
    /* Build PowerShell command to upload current directory (excluding .stem) */
    char command[2048];
    #ifdef _WIN32
    if (keep_backups > 0) {
        snprintf(command, sizeof(command),
            "powershell.exe -ExecutionPolicy Bypass -File \"%s\" -SourcePath \"%s\" -FolderId \"%s\" -KeepBackups %d",
            script_path, cwd, folder_id, keep_backups);
    } else {
        snprintf(command, sizeof(command),
            "powershell.exe -ExecutionPolicy Bypass -File \"%s\" -SourcePath \"%s\" -FolderId \"%s\"",
            script_path, cwd, folder_id);
    }
    #else
    if (keep_backups > 0) {
        snprintf(command, sizeof(command),
            "pwsh -File \"%s\" -SourcePath \"%s\" -FolderId \"%s\" -KeepBackups %d",
            script_path, cwd, folder_id, keep_backups);
    } else {
        snprintf(command, sizeof(command),
            "pwsh -File \"%s\" -SourcePath \"%s\" -FolderId \"%s\"",
            script_path, cwd, folder_id);
    }
    #endif
    
    printf("  Uploading repository data to Google Drive...\n");
    printf("\n");
    
    /* Execute PowerShell upload script */
    int result = system(command);
    
    printf("\n");
    
    if (result == 0) {
        print_success("Successfully pushed to '%s'", remote_name);
        printf("\n");
        print_info("Next steps:");
        printf("  - Your commits are backed up to Google Drive\n");
        printf("  - View in browser: https://drive.google.com/drive/folders/%s\n", folder_id);
        printf("  - Others can clone with: stem clone <your-gdrive-url>\n");
        printf("  - View status: stem status\n");
        return 0;
    } else if (result == 2) {
        /* Authorization needed */
        print_error("Google Drive authorization required");
        printf("\n");
        printf("This is a ONE-TIME setup. Run this command:\n\n");
        printf("  powershell -ExecutionPolicy Bypass -File \"%s\" -SetupAuth\n\n", script_path);
        printf("After authorization, run 'stem push' again.\n");
        printf("\n");
        printf("The authorization will be saved and you won't need to do this again.\n");
        return 1;
    } else {
        print_error("Push failed");
        printf("\n");
        printf("This is usually a network or upload error (not authorization).\n");
        printf("Check your connection and retry: stem push\n");
        printf("\nIf the script explicitly reports 401/403 auth errors, run:\n");
        printf("  powershell -ExecutionPolicy Bypass -File \"%s\" -SetupAuth\n", script_path);
        return 1;
    }
}

int cmd_pull(int argc, char **argv) {
    const char *remote_name = "cloud";
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: stem pull [remote] [options]\n\n");
            printf("Pull and sync from cloud storage\n\n");
            printf("Arguments:\n");
            printf("  remote         Remote name (default: cloud)\n\n");
            printf("Options:\n");
            printf("  --help         Show this help message\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            remote_name = argv[i];
        }
    }
    
    printf("\n");
    print_info("Pulling from remote '%s'...", remote_name);
    
    /* Check if .stem directory exists */
    if (!stem_path_exists(".stem")) {
        print_error("Not a STEM repository");
        printf("\nRun 'stem init' first to initialize a repository.\n");
        return 1;
    }
    
    /* Load remote configuration */
    FILE *config = fopen(".stem/config", "r");
    if (!config) {
        print_error("No remote configuration found");
        printf("\nAdd a remote with:\n");
        printf("  stem remote add %s <google-drive-url>\n", remote_name);
        return 1;
    }
    
    char folder_id[256] = {0};
    char provider[64] = {0};
    char line[512];
    bool in_remote_section = false;
    
    while (fgets(line, sizeof(line), config)) {
        line[strcspn(line, "\r\n")] = 0;
        
        if (strstr(line, "[remote \"cloud\"]")) {
            in_remote_section = true;
            continue;
        }
        
        if (in_remote_section) {
            if (strstr(line, "folderid")) {
                char *eq = strchr(line, '=');
                if (eq) {
                    eq++;
                    while (*eq == ' ') eq++;
                    strncpy(folder_id, eq, sizeof(folder_id) - 1);
                }
            }
            if (strstr(line, "provider")) {
                char *eq = strchr(line, '=');
                if (eq) {
                    eq++;
                    while (*eq == ' ') eq++;
                    strncpy(provider, eq, sizeof(provider) - 1);
                }
            }
        }
    }
    fclose(config);
    
    if (strlen(folder_id) == 0) {
        print_error("Google Drive folder ID not found in config");
        return 1;
    }
    
    printf("  Remote: %s (%s)\n", remote_name, provider);
    printf("  Folder: %s\n", folder_id);
    
    /* Get current working directory */
    char cwd[512];
    if (!getcwd(cwd, sizeof(cwd))) {
        print_error("Failed to get current directory");
        return 1;
    }
    
    printf("\n");
    print_info("Preparing download...");
    
    /* Get STEM installation directory */
    char script_path[512];
    #ifdef _WIN32
    char exe_path[512];
    GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
    char *last_slash = strrchr(exe_path, '\\');
    if (last_slash) *last_slash = '\0';
    last_slash = strrchr(exe_path, '\\');
    if (last_slash) *last_slash = '\0';
    snprintf(script_path, sizeof(script_path), "%s\\gdrive_upload.ps1", exe_path);
    #else
    strncpy(script_path, "/usr/local/share/stem/gdrive_upload.ps1", sizeof(script_path) - 1);
    #endif
    
    if (!stem_path_exists(script_path)) {
        print_error("Google Drive script not found");
        printf("\nExpected location: %s\n", script_path);
        return 1;
    }
    
    /* Build PowerShell command to pull */
    char command[2048];
    #ifdef _WIN32
    snprintf(command, sizeof(command),
        "powershell.exe -ExecutionPolicy Bypass -File \"%s\" -Pull -DestinationPath \"%s\" -FolderId \"%s\"",
        script_path, cwd, folder_id);
    #else
    snprintf(command, sizeof(command),
        "pwsh -File \"%s\" -Pull -DestinationPath \"%s\" -FolderId \"%s\"",
        script_path, cwd, folder_id);
    #endif
    
    printf("  Downloading from Google Drive...\n");
    printf("\n");
    
    /* Execute PowerShell pull script */
    int result = system(command);
    
    printf("\n");
    
    if (result == 0) {
        print_success("Successfully pulled from '%s'", remote_name);
        printf("\n");
        print_info("Next steps:");
        printf("  - Your files have been synced\n");
        printf("  - Check status: stem status\n");
        printf("  - View history: stem history\n");
        return 0;
    } else {
        print_error("Pull failed");
        printf("\nIf you see authorization errors, run:\n");
        printf("  powershell -ExecutionPolicy Bypass -File \"%s\" -SetupAuth\n", script_path);
        return 1;
    }
}

int cmd_sync(int argc, char **argv) {
    const char *remote_name = "cloud";
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: stem sync [remote] [options]\n\n");
            printf("Synchronize with cloud storage (pull + push)\n\n");
            printf("Arguments:\n");
            printf("  remote         Remote name (default: cloud)\n\n");
            printf("Options:\n");
            printf("  --help         Show this help message\n\n");
            printf("What happens:\n");
            printf("  1. Pull latest changes from cloud\n");
            printf("  2. Push local changes to cloud\n");
            printf("  3. Keep everything in sync\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            remote_name = argv[i];
        }
    }
    
    printf("\n");
    printf("================================================================\n");
    printf("  Synchronizing with '%s'\n", remote_name);
    printf("================================================================\n");
    printf("\n");
    
    /* Check if repository exists */
    if (!stem_path_exists(".stem")) {
        print_error("Not a STEM repository");
        printf("\nRun 'stem init' first to initialize a repository.\n");
        return 1;
    }
    
    /* Step 1: Pull */
    print_info("Step 1/2: Pulling from remote...");
    printf("\n");
    
    char *pull_args[] = {"pull", (char*)remote_name};
    int pull_result = cmd_pull(2, pull_args);
    
    if (pull_result != 0) {
        print_warning("Pull failed - continuing with push");
    }
    
    printf("\n");
    
    /* Step 2: Push */
    print_info("Step 2/2: Pushing to remote...");
    printf("\n");
    
    char *push_args[] = {"push", (char*)remote_name};
    int push_result = cmd_push(2, push_args);
    
    printf("\n");
    
    if (pull_result == 0 && push_result == 0) {
        print_success("Sync complete!");
        printf("\n");
        printf("Your local repository is now in sync with '%s'\n", remote_name);
    } else {
        print_warning("Sync completed with issues");
        printf("\n");
        if (pull_result != 0) printf("  - Pull had errors\n");
        if (push_result != 0) printf("  - Push had errors\n");
    }
    
    printf("\n");
    return (pull_result != 0 || push_result != 0) ? 1 : 0;
}

/* Note: cmd_history, cmd_diff, cmd_branch, cmd_switch, cmd_merge, cmd_restore, 
   and cmd_verify are now implemented in their respective files:
   - cmd_history.c: cmd_history, cmd_diff, cmd_restore, cmd_verify
   - cmd_branch.c: cmd_branch, cmd_switch
   - cmd_merge.c: cmd_merge
*/

/* Config command - manage repository settings */
int cmd_config(int argc, char **argv) {
    const char *key = NULL;
    const char *value = NULL;
    int list_mode = 0;
    int global_mode = 0;
    
    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--list") == 0 || strcmp(argv[i], "-l") == 0) {
            list_mode = 1;
        }
        else if (strcmp(argv[i], "--global") == 0) {
            global_mode = 1;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("\n");
            printf("Usage: stem config [options] <key> [value]\n");
            printf("\n");
            printf("Manage repository configuration\n");
            printf("\n");
            printf("Options:\n");
            printf("  --list, -l     List all config values\n");
            printf("  --global       Use global config (user-level)\n");
            printf("  --help         Show this help\n");
            printf("\n");
            printf("Common settings:\n");
            printf("  user.name      Your name\n");
            printf("  user.email     Your email\n");
            printf("  core.editor    Text editor to use\n");
            printf("  cloud.keep_backups  Number of cloud backups to keep\n");
            printf("\n");
            printf("Examples:\n");
            printf("  stem config user.name \"John Doe\"\n");
            printf("  stem config user.email \"john@example.com\"\n");
            printf("  stem config cloud.keep_backups 5\n");
            printf("  stem config --list\n");
            printf("\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            if (!key) {
                key = argv[i];
            } else {
                value = argv[i];
            }
        }
    }
    
    /* Determine config file path */
    char config_path[512];
    if (global_mode) {
        snprintf(config_path, sizeof(config_path), "%s/.stemconfig", getenv("HOME") ? getenv("HOME") : getenv("USERPROFILE"));
    } else {
        if (!stem_path_exists(".stem")) {
            print_error("Not a STEM repository");
            printf("\nRun 'stem init' first or use --global for user-level config\n");
            return 1;
        }
        snprintf(config_path, sizeof(config_path), ".stem/config");
    }
    
    /* List mode */
    if (list_mode) {
        FILE *config = fopen(config_path, "r");
        if (!config) {
            print_warning("No configuration found");
            return 0;
        }
        
        printf("\n");
        printf("Configuration (%s):\n", global_mode ? "global" : "local");
        printf("\n");
        
        char line[512];
        while (fgets(line, sizeof(line), config)) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) > 0 && line[0] != '[' && line[0] != '#') {
                printf("  %s\n", line);
            }
        }
        
        fclose(config);
        printf("\n");
        return 0;
    }
    
    /* Get value */
    if (key && !value) {
        FILE *config = fopen(config_path, "r");
        if (!config) {
            printf("(not set)\n");
            return 1;
        }
        
        char line[512];
        int found = 0;
        while (fgets(line, sizeof(line), config)) {
            line[strcspn(line, "\r\n")] = 0;
            char *eq = strchr(line, '=');
            if (eq) {
                *eq = '\0';
                char *line_key = line;
                while (*line_key == ' ' || *line_key == '\t') line_key++;
                
                if (strcmp(line_key, key) == 0) {
                    char *val = eq + 1;
                    while (*val == ' ' || *val == '\t') val++;
                    printf("%s\n", val);
                    found = 1;
                    break;
                }
            }
        }
        
        fclose(config);
        if (!found) {
            printf("(not set)\n");
            return 1;
        }
        return 0;
    }
    
    /* Set value */
    if (key && value) {
        /* Read existing config */
        FILE *config = fopen(config_path, "r");
        char **lines = NULL;
        int line_count = 0;
        int found = 0;
        
        if (config) {
            char line[512];
            while (fgets(line, sizeof(line), config)) {
                line[strcspn(line, "\r\n")] = 0;
                
                /* Check if this is the key we're updating */
                char *eq = strchr(line, '=');
                if (eq) {
                    char temp[512];
                    strncpy(temp, line, eq - line);
                    temp[eq - line] = '\0';
                    
                    char *line_key = temp;
                    while (*line_key == ' ' || *line_key == '\t') line_key++;
                    
                    if (strcmp(line_key, key) == 0) {
                        /* Update this line */
                        snprintf(line, sizeof(line), "%s = %s", key, value);
                        found = 1;
                    }
                }
                
                lines = realloc(lines, sizeof(char*) * (line_count + 1));
                lines[line_count] = strdup(line);
                line_count++;
            }
            fclose(config);
        }
        
        /* Add new key if not found */
        if (!found) {
            lines = realloc(lines, sizeof(char*) * (line_count + 1));
            char new_line[512];
            snprintf(new_line, sizeof(new_line), "%s = %s", key, value);
            lines[line_count] = strdup(new_line);
            line_count++;
        }
        
        /* Write back */
        config = fopen(config_path, "w");
        if (!config) {
            print_error("Could not write config file");
            return 1;
        }
        
        for (int i = 0; i < line_count; i++) {
            fprintf(config, "%s\n", lines[i]);
            free(lines[i]);
        }
        free(lines);
        
        fclose(config);
        
        print_success("Config updated: %s = %s", key, value);
        printf("\n");
        return 0;
    }
    
    print_error("Please specify a key");
    printf("\nUsage: stem config <key> [value]\n");
    printf("       stem config --list\n");
    return 1;
}

int cmd_doctor(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("\n");
    printf("================================================================\n");
    printf("  STEM VCS - Repository Diagnostics\n");
    printf("================================================================\n");
    printf("\n");
    
    int issues = 0;
    int warnings = 0;
    
    /* Check if in repository */
    printf("[*] Checking repository...\n");
    if (!stem_path_exists(".stem")) {
        print_error("  Not a STEM repository");
        printf("\n");
        printf("Run 'stem init' to initialize a repository\n");
        return 1;
    }
    print_success("  Repository detected");
    
    /* Check HEAD */
    printf("[*] Checking HEAD...\n");
    FILE *head = fopen(".stem/HEAD", "r");
    if (head) {
        print_success("  HEAD file exists");
        fclose(head);
    } else {
        print_error("  HEAD file missing");
        issues++;
    }
    
    /* Check refs structure */
    printf("[*] Checking references...\n");
    if (stem_path_exists(".stem/refs") && 
        stem_path_exists(".stem/refs/heads")) {
        print_success("  Reference structure OK");
    } else {
        print_warning("  Reference directories missing");
        warnings++;
    }
    
    /* Check objects */
    printf("[*] Checking object store...\n");
    if (stem_path_exists(".stem/objects")) {
        print_success("  Object store exists");
        
        /* Count objects */
        int commit_count = 0;
        if (stem_path_exists(".stem/objects/commits")) {
            #ifdef _WIN32
            WIN32_FIND_DATAA find_data;
            HANDLE hFind = FindFirstFileA(".stem/objects/commits/*", &find_data);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (strcmp(find_data.cFileName, ".") != 0 && 
                        strcmp(find_data.cFileName, "..") != 0) {
                        commit_count++;
                    }
                } while (FindNextFileA(hFind, &find_data));
                FindClose(hFind);
            }
            #endif
        }
        printf("    Commits: %d\n", commit_count);
    } else {
        print_warning("  Object store missing");
        warnings++;
    }
    
    /* Check index */
    printf("[*] Checking index...\n");
    FILE *index = fopen(".stem/index", "r");
    if (index) {
        int staged_files = 0;
        char line[2048];
        while (fgets(line, sizeof(line), index)) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) > 0) staged_files++;
        }
        fclose(index);
        print_success("  Index OK");
        printf("    Staged files: %d\n", staged_files);
    } else {
        print_warning("  Index file not found (normal for new repo)");
    }
    
    /* Check remote configuration */
    printf("[*] Checking remote...\n");
    FILE *config = fopen(".stem/config", "r");
    if (config) {
        char line[512];
        int has_remote = 0;
        while (fgets(line, sizeof(line), config)) {
            if (strstr(line, "[remote")) {
                has_remote = 1;
                break;
            }
        }
        fclose(config);
        
        if (has_remote) {
            print_success("  Remote configured");
        } else {
            print_warning("  No remote configured");
            printf("    Add remote: stem remote add cloud <gdrive-url>\n");
            warnings++;
        }
    } else {
        print_warning("  No configuration file");
        warnings++;
    }
    
    /* Check disk space */
    printf("[*] Checking disk space...\n");
    #ifdef _WIN32
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);
    char drive[4] = {cwd[0], ':', '\\', '\0'};
    
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    if (GetDiskFreeSpaceExA(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        double freeGB = (double)freeBytesAvailable.QuadPart / (1024.0 * 1024.0 * 1024.0);
        if (freeGB < 1.0) {
            print_error("  Low disk space: %.2f GB", freeGB);
            issues++;
        } else if (freeGB < 5.0) {
            print_warning("  Disk space: %.2f GB", freeGB);
            warnings++;
        } else {
            print_success("  Disk space: %.2f GB", freeGB);
        }
    }
    #endif
    
    /* Check .stemignore */
    printf("[*] Checking .stemignore...\n");
    if (stem_path_exists(".stemignore")) {
        print_success("  .stemignore exists");
    } else {
        print_warning("  No .stemignore file");
        printf("    Run 'stem add .' to create one\n");
        warnings++;
    }
    
    /* Check Google Drive connectivity */
    printf("[*] Checking Google Drive setup...\n");
    char token_path[512];
    snprintf(token_path, sizeof(token_path), "%s\\.stem_gdrive_token", 
             getenv("USERPROFILE") ? getenv("USERPROFILE") : getenv("HOME"));
    
    if (stem_path_exists(token_path)) {
        print_success("  Google Drive token found");
    } else {
        print_warning("  No Google Drive authorization");
        printf("    Run: powershell -ExecutionPolicy Bypass -File gdrive_upload.ps1 -SetupAuth\n");
        warnings++;
    }
    
    printf("\n");
    printf("================================================================\n");
    printf("  Diagnostic Summary\n");
    printf("================================================================\n");
    printf("\n");
    
    if (issues == 0 && warnings == 0) {
        print_success("All checks passed!");
        printf("\n");
        printf("Your repository is healthy and ready to use.\n");
    } else {
        if (issues > 0) {
            print_error("%d critical issue(s) found", issues);
        }
        if (warnings > 0) {
            print_warning("%d warning(s)", warnings);
        }
        printf("\n");
        printf("Review the output above for details.\n");
    }
    
    printf("\n");
    return issues > 0 ? 1 : 0;
}
