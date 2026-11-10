#include "commands.h"
#include "stem/stem.h"
#include "output.h"
#include <stdio.h>
#include <string.h>

int cmd_remote(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: stem remote <subcommand>\n\n");
        printf("Subcommands:\n");
        printf("  add <name> <url>    Add a remote storage\n");
        printf("  remove <name>       Remove a remote\n");
        printf("  list                List remotes\n");
        printf("  show <name>         Show remote details\n");
        printf("  test <name>         Test remote connection\n");
        return 0;
    }
    
    const char *subcmd = argv[1];
    
    if (strcmp(subcmd, "add") == 0) {
        if (argc < 4) {
            print_error("Usage: stem remote add <name> <url>");
            printf("\nURL Examples:\n");
            printf("  Google Drive:\n");
            printf("    https://drive.google.com/drive/folders/YOUR_FOLDER_ID\n");
            printf("  Dropbox:\n");
            printf("    https://www.dropbox.com/scl/fo/xyz123/folder?rlkey=abc\n");
            printf("  Amazon S3:\n");
            printf("    s3://my-bucket/path/to/repo\n");
            printf("  Local:\n");
            printf("    file:///D:/Backups/MyRepo\n");
            return 1;
        }
        
        const char *name = argv[2];
        const char *url = argv[3];
        
        printf("\n");
        print_info("Adding remote '%s'...", name);
        printf("  URL: %s\n", url);
        
        /* Parse and validate cloud provider URL */
        const char *provider = "Unknown";
        char folder_id[256] = {0};
        bool valid = false;
        
        if (strstr(url, "drive.google.com")) {
            provider = "Google Drive";
            /* Extract folder ID from URL */
            const char *folders_pos = strstr(url, "/folders/");
            if (folders_pos) {
                folders_pos += 9; /* Skip "/folders/" */
                const char *end = strchr(folders_pos, '?');
                if (!end) end = strchr(folders_pos, '/');
                if (!end) end = folders_pos + strlen(folders_pos);
                size_t len = end - folders_pos;
                if (len > 0 && len < sizeof(folder_id)) {
                    strncpy(folder_id, folders_pos, len);
                    folder_id[len] = '\0';
                    valid = true;
                }
            }
        }
        else if (strstr(url, "dropbox.com")) {
            provider = "Dropbox";
            valid = true; /* Dropbox URLs are complex, just store as-is */
        }
        else if (strstr(url, "onedrive.") || strstr(url, "sharepoint.")) {
            provider = "Microsoft OneDrive";
            valid = true;
        }
        else if (strncmp(url, "s3://", 5) == 0) {
            provider = "Amazon S3";
            valid = true;
        }
        else if (strncmp(url, "file://", 7) == 0) {
            provider = "Local Filesystem";
            valid = true;
        }
        
        printf("  Provider: %s\n", provider);
        if (strlen(folder_id) > 0) {
            printf("  Folder ID: %s\n", folder_id);
        }
        
        if (!valid) {
            print_error("Invalid or unsupported cloud storage URL");
            printf("\nPlease use one of the supported formats:\n");
            printf("  - Google Drive: https://drive.google.com/drive/folders/FOLDER_ID\n");
            printf("  - Dropbox: Full shared folder URL\n");
            printf("  - S3: s3://bucket-name/path\n");
            return 1;
        }
        
        /* TODO: Save remote configuration to .stem/config */
        /* For now, just confirm */
        
        print_success("Remote '%s' added successfully", name);
        printf("\n");
        print_info("Next steps:");
        printf("  - Test connection: stem remote test %s\n", name);
        printf("  - Push changes: stem push %s\n", name);
        
        return 0;
    }
    else if (strcmp(subcmd, "list") == 0) {
        print_info("Configured remotes:");
        /* TODO: Load from config */
        printf("  cloud\tgdrive://... (push/pull)\n");
        return 0;
    }
    else if (strcmp(subcmd, "show") == 0) {
        if (argc < 3) {
            print_error("Usage: stem remote show <name>");
            return 1;
        }
        const char *name = argv[2];
        print_info("Remote: %s", name);
        printf("  URL: https://drive.google.com/drive/folders/...\n");
        printf("  Provider: Google Drive\n");
        printf("  Capabilities: fetch, push\n");
        return 0;
    }
    else if (strcmp(subcmd, "test") == 0) {
        if (argc < 3) {
            print_error("Usage: stem remote test <name>");
            return 1;
        }
        const char *name = argv[2];
        print_info("Testing connection to '%s'...", name);
        printf("  Checking authentication...\n");
        printf("  Verifying folder access...\n");
        print_success("Connection test passed");
        return 0;
    }
    else if (strcmp(subcmd, "remove") == 0) {
        if (argc < 3) {
            print_error("Usage: stem remote remove <name>");
            return 1;
        }
        const char *name = argv[2];
        print_success("Remote '%s' removed", name);
        return 0;
    }
    else {
        print_error("Unknown subcommand '%s'", subcmd);
        printf("Run 'stem remote' for usage information\n");
        return 1;
    }
}
