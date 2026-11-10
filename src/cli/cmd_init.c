#include "commands.h"
#include "stem/stem.h"
#include "output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

int cmd_init(int argc, char **argv) {
    char target_dir[1024];
    char cloud_url[512] = {0};
    bool bare = false;
    bool interactive = false;
    
    /* Parse options */
    const char *path_arg = NULL;
    const char *cloud_arg = NULL;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--bare") == 0) {
            bare = true;
        }
        else if (strcmp(argv[i], "--path") == 0 && i + 1 < argc) {
            path_arg = argv[++i];
        }
        else if (strcmp(argv[i], "--cloud") == 0 && i + 1 < argc) {
            cloud_arg = argv[++i];
        }
        else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
            interactive = true;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: stem init [options]\n\n");
            printf("Initialize a new STEM repository\n\n");
            printf("Options:\n");
            printf("  --path <dir>       Target directory (default: current)\n");
            printf("  --cloud <url>      Cloud storage URL\n");
            printf("  -i, --interactive  Interactive setup\n");
            printf("  --bare             Initialize as bare repository\n");
            printf("  --help             Show this help message\n\n");
            printf("Cloud URL Examples:\n");
            printf("  https://drive.google.com/drive/folders/ABC123xyz\n");
            printf("  dropbox://Projects/MyRepo\n");
            printf("  s3://my-bucket/myrepo\n\n");
            printf("Examples:\n");
            printf("  stem init --path C:\\Projects\\MyRepo --cloud <gdrive-url>\n");
            printf("  stem init -i\n");
            return 0;
        }
    }
    
    /* Interactive mode */
    if (interactive || (!path_arg && !cloud_arg)) {
        printf("\n");
        print_info("STEM Repository Setup");
        printf("\n");
        
        /* Get directory */
        printf("Repository location (press Enter for current directory):\n> ");
        fgets(target_dir, sizeof(target_dir), stdin);
        target_dir[strcspn(target_dir, "\n")] = 0;
        
        if (strlen(target_dir) == 0) {
            if (!getcwd(target_dir, sizeof(target_dir))) {
                print_error("Failed to get current directory");
                return 1;
            }
        }
        
        /* Get cloud URL */
        printf("\nCloud storage URL (required):\n");
        printf("Examples:\n");
        printf("  - Google Drive: https://drive.google.com/drive/folders/YOUR_FOLDER_ID\n");
        printf("  - Dropbox: https://www.dropbox.com/scl/fo/YOUR_FOLDER_ID\n");
        printf("> ");
        fgets(cloud_url, sizeof(cloud_url), stdin);
        cloud_url[strcspn(cloud_url, "\n")] = 0;
        
        if (strlen(cloud_url) == 0) {
            print_error("Cloud storage URL is required");
            printf("\nYou can add it later with:\n");
            printf("  stem remote add cloud <url>\n");
            return 1;
        }
    } else {
        /* Non-interactive mode */
        if (path_arg) {
            strncpy(target_dir, path_arg, sizeof(target_dir) - 1);
        } else {
            if (!getcwd(target_dir, sizeof(target_dir))) {
                print_error("Failed to get current directory");
                return 1;
            }
        }
        
        if (cloud_arg) {
            strncpy(cloud_url, cloud_arg, sizeof(cloud_url) - 1);
        }
    }
    
    printf("\n");
    print_info("Initializing STEM repository...");
    printf("  Location: %s\n", target_dir);
    if (strlen(cloud_url) > 0) {
        printf("  Cloud:    %s\n", cloud_url);
    }
    
    /* Initialize repository */
    stem_repository_t *repo = NULL;
    stem_error_t err = stem_repository_init(target_dir, bare, &repo);
    
    if (err != STEM_OK) {
        print_error("Failed to initialize repository: %s", stem_error_message(err));
        return 1;
    }
    
    /* Add cloud remote if provided */
    if (strlen(cloud_url) > 0) {
        print_info("Configuring cloud storage...");
        
        /* Save cloud URL to config */
        char config_path[2048];
        snprintf(config_path, sizeof(config_path), "%s/.stem/config", target_dir);
        
        FILE *config = fopen(config_path, "w");
        if (config) {
            fprintf(config, "[remote \"cloud\"]\n");
            fprintf(config, "    url = %s\n", cloud_url);
            fprintf(config, "    fetch = +refs/heads/*:refs/remotes/cloud/*\n");
            fprintf(config, "    provider = gdrive\n");
            
            /* Extract folder ID from Google Drive URL */
            const char *folders_pos = strstr(cloud_url, "/folders/");
            if (folders_pos) {
                folders_pos += 9;
                const char *end = strchr(folders_pos, '?');
                if (!end) end = strchr(folders_pos, '/');
                if (!end) end = folders_pos + strlen(folders_pos);
                char folder_id[256] = {0};
                size_t len = end - folders_pos;
                if (len > 0 && len < sizeof(folder_id)) {
                    strncpy(folder_id, folders_pos, len);
                    folder_id[len] = '\0';
                    fprintf(config, "    folderid = %s\n", folder_id);
                }
            }
            
            fclose(config);
        }
    }
    
    printf("\n");
    print_success("Repository initialized successfully!");
    
    if (!bare) {
        printf("\n");
        print_info("Next steps:");
        printf("  1. Navigate: cd \"%s\"\n", target_dir);
        if (strlen(cloud_url) == 0) {
            printf("  2. Add cloud: stem remote add cloud <url>\n");
            printf("  3. Add files: stem add .\n");
        } else {
            printf("  2. Add files: stem add .\n");
        }
        printf("  %d. Commit:    stem commit -m \"Initial commit\"\n", strlen(cloud_url) == 0 ? 4 : 3);
        printf("  %d. Push:      stem push\n", strlen(cloud_url) == 0 ? 5 : 4);
    }
    
    stem_repository_free(repo);
    
    return 0;
}
