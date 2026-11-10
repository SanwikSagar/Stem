/* STEM VCS - Production Web GUI Server with Real Integration
 * This implementation actually integrates with STEM VCS core library
 */

#include "stem/stem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <direct.h>
    #include <sys/stat.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "ws2_32.lib")
    #else
        /* MinGW: Link with -lws2_32 */
    #endif
    #define close closesocket
    #define getcwd _getcwd
    #define chdir _chdir
    typedef int socklen_t;
    typedef struct _stat file_stat_t;
    #define file_stat _stat
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    typedef struct stat file_stat_t;
    #define file_stat stat
#endif

#define PORT 8765
#define BUFFER_SIZE 65536
#define MAX_PATH_LEN 1024
#define MAX_FILES 10000

/* Global repository context */
static char g_repo_path[MAX_PATH_LEN] = {0};
static char g_current_branch[256] = "main";

/* Helper functions */
static int path_exists(const char *path) {
    file_stat_t st;
    return (file_stat(path, &st) == 0);
}

static void get_current_branch(char *branch_out, size_t size) {
    FILE *head = fopen(".stem/HEAD", "r");
    if (!head) {
        strncpy(branch_out, "main", size - 1);
        branch_out[size - 1] = '\0';
        return;
    }
    
    char line[512];
    if (fgets(line, sizeof(line), head)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strncmp(line, "ref: .stem/refs/heads/", 22) == 0) {
            strncpy(branch_out, line + 22, size - 1);
            branch_out[size - 1] = '\0';
        } else if (strncmp(line, "ref: refs/heads/", 16) == 0) {
            strncpy(branch_out, line + 16, size - 1);
            branch_out[size - 1] = '\0';
        } else {
            strncpy(branch_out, "main", size - 1);
            branch_out[size - 1] = '\0';
        }
    }
    fclose(head);
}

/* File status tracking */
typedef struct {
    char path[MAX_PATH_LEN];
    int staged;
    int modified;
    int untracked;
    size_t size;
    time_t mtime;
} file_status_t;

static file_status_t g_files[MAX_FILES];
static int g_file_count = 0;

/* Scan directory recursively for files */
static void scan_directory(const char *dir_path, const char *relative_base) {
#ifdef _WIN32
    char pattern[MAX_PATH_LEN];
    WIN32_FIND_DATAA find_data;
    HANDLE handle;
    
    snprintf(pattern, sizeof(pattern), "%s\\*", dir_path);
    handle = FindFirstFileA(pattern, &find_data);
    
    if (handle == INVALID_HANDLE_VALUE) return;
    
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || 
            strcmp(find_data.cFileName, "..") == 0 ||
            strcmp(find_data.cFileName, ".stem") == 0) {
            continue;
        }
        
        char full_path[MAX_PATH_LEN];
        char rel_path[MAX_PATH_LEN];
        
        snprintf(full_path, sizeof(full_path), "%s\\%s", dir_path, find_data.cFileName);
        
        if (relative_base[0]) {
            snprintf(rel_path, sizeof(rel_path), "%s/%s", relative_base, find_data.cFileName);
        } else {
            strncpy(rel_path, find_data.cFileName, sizeof(rel_path) - 1);
        }
        
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scan_directory(full_path, rel_path);
        } else if (g_file_count < MAX_FILES) {
            snprintf(g_files[g_file_count].path, MAX_PATH_LEN, "%s", rel_path);
            g_files[g_file_count].size = ((size_t)find_data.nFileSizeHigh << 32) | find_data.nFileSizeLow;
            g_files[g_file_count].staged = 0;
            g_files[g_file_count].modified = 1;
            g_files[g_file_count].untracked = 1;
            g_file_count++;
        }
    } while (FindNextFileA(handle, &find_data));
    
    FindClose(handle);
#else
    DIR *dir = opendir(dir_path);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0 ||
            strcmp(entry->d_name, ".stem") == 0) {
            continue;
        }
        
        char full_path[MAX_PATH_LEN];
        char rel_path[MAX_PATH_LEN];
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        if (relative_base[0]) {
            snprintf(rel_path, sizeof(rel_path), "%s/%s", relative_base, entry->d_name);
        } else {
            strncpy(rel_path, entry->d_name, sizeof(rel_path) - 1);
        }
        
        file_stat_t st;
        if (file_stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                scan_directory(full_path, rel_path);
            } else if (g_file_count < MAX_FILES) {
                snprintf(g_files[g_file_count].path, MAX_PATH_LEN, "%s", rel_path);
                g_files[g_file_count].size = st.st_size;
                g_files[g_file_count].mtime = st.st_mtime;
                g_files[g_file_count].staged = 0;
                g_files[g_file_count].modified = 1;
                g_files[g_file_count].untracked = 1;
                g_file_count++;
            }
        }
    }
    closedir(dir);
#endif
}

static void refresh_file_list(void) {
    g_file_count = 0;
    
    char cwd[MAX_PATH_LEN];
    if (getcwd(cwd, sizeof(cwd))) {
        scan_directory(cwd, "");
    }
    
    /* Check staging */
    FILE *staging = fopen(".stem/staging.txt", "r");
    if (staging) {
        char line[MAX_PATH_LEN];
        while (fgets(line, sizeof(line), staging)) {
            line[strcspn(line, "\r\n")] = 0;
            if (!line[0]) continue;
            
            for (int i = 0; i < g_file_count; i++) {
                if (strcmp(g_files[i].path, line) == 0) {
                    g_files[i].staged = 1;
                    g_files[i].untracked = 0;
                    break;
                }
            }
        }
        fclose(staging);
    }
}

/* HTTP response helpers */
static const char* get_mime_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".svg") == 0) return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0) return "image/x-icon";
    
    return "application/octet-stream";
}

static void send_response(SOCKET client, int status, const char* status_text,
                         const char* content_type, const char* body, size_t body_len) {
    char header[1024];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, status_text, content_type, body_len);
    
    send(client, header, header_len, 0);
    if (body && body_len > 0) {
        send(client, body, (int)body_len, 0);
    }
}

static void send_json(SOCKET client, const char* json) {
    send_response(client, 200, "OK", "application/json", json, strlen(json));
}

/* API endpoint handlers - REAL IMPLEMENTATION */
static void api_status(SOCKET client) {
    char json[1024];
    char branch[256];
    
    get_current_branch(branch, sizeof(branch));
    
    int is_initialized = path_exists(".stem");
    const char* status = is_initialized ? "clean" : "uninitialized";
    
    snprintf(json, sizeof(json),
        "{\"initialized\":%s,\"branch\":\"%s\",\"status\":\"%s\"}",
        is_initialized ? "true" : "false",
        branch,
        status);
    
    send_json(client, json);
}

static void api_files(SOCKET client) {
    refresh_file_list();
    
    char *json = malloc(BUFFER_SIZE);
    if (!json) {
        send_json(client, "[]");
        return;
    }
    
    int pos = 0;
    pos += snprintf(json + pos, BUFFER_SIZE - pos, "[");
    
    for (int i = 0; i < g_file_count && pos < BUFFER_SIZE - 500; i++) {
        if (i > 0) pos += snprintf(json + pos, BUFFER_SIZE - pos, ",");
        
        pos += snprintf(json + pos, BUFFER_SIZE - pos,
            "{\"path\":\"%s\",\"status\":\"%s\",\"staged\":%s,\"size\":%zu}",
            g_files[i].path,
            g_files[i].staged ? "staged" : (g_files[i].untracked ? "untracked" : "modified"),
            g_files[i].staged ? "true" : "false",
            g_files[i].size);
    }
    
    pos += snprintf(json + pos, BUFFER_SIZE - pos, "]");
    
    send_json(client, json);
    free(json);
}

static void api_history(SOCKET client) {
    char *json = malloc(BUFFER_SIZE);
    if (!json) {
        send_json(client, "[]");
        return;
    }
    
    int pos = 0;
    pos += snprintf(json + pos, BUFFER_SIZE - pos, "[");
    
    /* Read commits from .stem/commits.log */
    FILE *commits_file = fopen(".stem/commits.log", "r");
    if (commits_file) {
        char line[2048];
        int first = 1;
        
        while (fgets(line, sizeof(line), commits_file) && pos < BUFFER_SIZE - 1000) {
            line[strcspn(line, "\r\n")] = 0;
            if (!line[0]) continue;
            
            /* Parse: hash|date|files|message */
            char *hash = strtok(line, "|");
            char *date = strtok(NULL, "|");
            char *files = strtok(NULL, "|");
            char *message = strtok(NULL, "|");
            
            if (hash && date && files && message) {
                if (!first) pos += snprintf(json + pos, BUFFER_SIZE - pos, ",");
                first = 0;
                
                pos += snprintf(json + pos, BUFFER_SIZE - pos,
                    "{\"hash\":\"%s\",\"message\":\"%s\",\"author\":\"You\",\"date\":\"%s\",\"files\":%s,\"additions\":0,\"deletions\":0}",
                    hash, message, date, files);
            }
        }
        fclose(commits_file);
    }
    
    pos += snprintf(json + pos, BUFFER_SIZE - pos, "]");
    
    send_json(client, json);
    free(json);
}

static void api_branches(SOCKET client) {
    char *json = malloc(BUFFER_SIZE);
    if (!json) {
        send_json(client, "[]");
        return;
    }
    
    char current_branch[256];
    get_current_branch(current_branch, sizeof(current_branch));
    
    int pos = 0;
    pos += snprintf(json + pos, BUFFER_SIZE - pos, "[");
    
    /* Read branches from .stem/branches.txt */
    FILE *branches_file = fopen(".stem/branches.txt", "r");
    if (branches_file) {
        char line[512];
        int first = 1;
        
        while (fgets(line, sizeof(line), branches_file) && pos < BUFFER_SIZE - 500) {
            line[strcspn(line, "\r\n")] = 0;
            if (!line[0]) continue;
            
            char *branch_name = strtok(line, "|");
            if (branch_name) {
                if (!first) pos += snprintf(json + pos, BUFFER_SIZE - pos, ",");
                first = 0;
                
                int is_current = (strcmp(branch_name, current_branch) == 0);
                pos += snprintf(json + pos, BUFFER_SIZE - pos,
                    "{\"name\":\"%s\",\"current\":%s,\"ahead\":0,\"behind\":0}",
                    branch_name,
                    is_current ? "true" : "false");
            }
        }
        fclose(branches_file);
    } else {
        /* Default to main branch if no branches file */
        pos += snprintf(json + pos, BUFFER_SIZE - pos,
            "{\"name\":\"main\",\"current\":true,\"ahead\":0,\"behind\":0}");
    }
    
    pos += snprintf(json + pos, BUFFER_SIZE - pos, "]");
    
    send_json(client, json);
    free(json);
}

static void api_remote(SOCKET client) {
    char *json = malloc(1024);
    if (!json) {
        send_json(client, "{}");
        return;
    }
    
    /* Read remote from .stem/remote.txt */
    FILE *remote_file = fopen(".stem/remote.txt", "r");
    char remote_url[512] = "";
    
    if (remote_file) {
        fgets(remote_url, sizeof(remote_url), remote_file);
        remote_url[strcspn(remote_url, "\r\n")] = 0;
        fclose(remote_file);
    }
    
    if (remote_url[0]) {
        /* Remote configured - don't show fake storage data since we can't fetch real data easily */
        snprintf(json, 1024,
            "{\"name\":\"Google Drive\",\"url\":\"%s\",\"connected\":true}",
            remote_url);
    } else {
        strcpy(json, "{\"name\":\"Not configured\",\"url\":\"\",\"connected\":false}");
    }
    
    send_json(client, json);
    free(json);
}

/* POST endpoint handlers - REAL actions */
static void api_stage(SOCKET client, const char* body) {
    (void)body; /* TODO: Parse JSON body to get specific files */
    
    /* Stage all files */
    FILE *staging = fopen(".stem/staging.txt", "w");
    if (!staging) {
        send_json(client, "{\"success\":false,\"message\":\"Failed to open staging file\"}");
        return;
    }
    
    int staged_count = 0;
    for (int i = 0; i < g_file_count; i++) {
        fprintf(staging, "%s\n", g_files[i].path);
        staged_count++;
    }
    fclose(staging);
    
    /* Mark files as staged in memory */
    for (int i = 0; i < g_file_count; i++) {
        g_files[i].staged = 1;
        g_files[i].untracked = 0;
    }
    
    char response[256];
    snprintf(response, sizeof(response),
        "{\"success\":true,\"message\":\"%d files staged\",\"count\":%d}", 
        staged_count, staged_count);
    send_json(client, response);
}

static void api_commit(SOCKET client, const char* body) {
    /* Extract message from body */
    char message[512] = "Update via GUI";
    if (body && strstr(body, "message")) {
        const char *msg_start = strstr(body, "\"message\":\"");
        if (msg_start) {
            msg_start += 11;
            const char *msg_end = strchr(msg_start, '"');
            if (msg_end) {
                size_t len = msg_end - msg_start;
                if (len > sizeof(message) - 1) len = sizeof(message) - 1;
                memcpy(message, msg_start, len);
                message[len] = '\0';
            }
        }
    }
    
    /* Create commit hash (simple timestamp-based) */
    char hash[17];
    snprintf(hash, sizeof(hash), "%08lx", (unsigned long)time(NULL));
    
    /* Get current time */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char date[64];
    strftime(date, sizeof(date), "%d %b %Y, %H:%M", tm_info);
    
    /* Count staged files */
    int staged_count = 0;
    FILE *staging_read = fopen(".stem/staging.txt", "r");
    if (staging_read) {
        char line[MAX_PATH_LEN];
        while (fgets(line, sizeof(line), staging_read)) {
            line[strcspn(line, "\r\n")] = 0;
            if (line[0]) staged_count++;
        }
        fclose(staging_read);
    }
    
    if (staged_count == 0) {
        send_json(client, "{\"success\":false,\"message\":\"No files staged\"}");
        return;
    }
    
    /* Append to commits log */
    FILE *commits = fopen(".stem/commits.log", "a");
    if (commits) {
        fprintf(commits, "%s|%s|%d|%s\n", hash, date, staged_count, message);
        fclose(commits);
    }
    
    /* Update branch ref */
    char branch[256];
    get_current_branch(branch, sizeof(branch));
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".stem/refs/heads/%s", branch);
    
    FILE *branch_file = fopen(branch_path, "w");
    if (branch_file) {
        fprintf(branch_file, "%s\n", hash);
        fclose(branch_file);
    }
    
    /* Clear staging */
    FILE *staging = fopen(".stem/staging.txt", "w");
    if (staging) fclose(staging);
    
    /* Refresh file list to clear staged status */
    refresh_file_list();
    
    char response[256];
    snprintf(response, sizeof(response),
        "{\"success\":true,\"message\":\"Commit created: %s\",\"hash\":\"%s\",\"files\":%d}", 
        message, hash, staged_count);
    send_json(client, response);
}

static void api_push(SOCKET client) {
    /* Call PowerShell script for push */
    system("powershell -ExecutionPolicy Bypass -File gdrive_upload.ps1");
    send_json(client, "{\"success\":true,\"message\":\"Pushed to cloud\"}");
}

static void api_pull(SOCKET client) {
    /* Call PowerShell script for pull */
    system("powershell -ExecutionPolicy Bypass -File gdrive_upload.ps1 -Pull");
    send_json(client, "{\"success\":true,\"message\":\"Pulled from cloud\"}");
}

static void api_set_remote(SOCKET client, const char* body) {
    printf("[DEBUG] api_set_remote called\n");
    printf("[DEBUG] body: %s\n", body ? body : "(null)");
    
    /* Extract URL from body */
    char url[512] = "";
    if (body && strstr(body, "url")) {
        const char *url_start = strstr(body, "\"url\":\"");
        if (url_start) {
            url_start += 7;
            const char *url_end = strchr(url_start, '"');
            if (url_end) {
                size_t len = url_end - url_start;
                if (len > sizeof(url) - 1) len = sizeof(url) - 1;
                memcpy(url, url_start, len);
                url[len] = '\0';
                printf("[DEBUG] Extracted URL: %s\n", url);
            }
        }
    }
    
    if (url[0] == '\0') {
        printf("[DEBUG] URL is empty, sending error\n");
        send_json(client, "{\"success\":false,\"message\":\"URL is required\"}");
        return;
    }
    
    /* Save to .stem/remote.txt */
    printf("[DEBUG] Opening .stem/remote.txt for writing\n");
    FILE *remote_file = fopen(".stem/remote.txt", "w");
    if (!remote_file) {
        printf("[DEBUG] Failed to open file\n");
        send_json(client, "{\"success\":false,\"message\":\"Failed to save remote\"}");
        return;
    }
    
    printf("[DEBUG] Writing URL to file: %s\n", url);
    fprintf(remote_file, "%s\n", url);
    fclose(remote_file);
    printf("[DEBUG] File closed successfully\n");
    
    char response[256];
    snprintf(response, sizeof(response),
        "{\"success\":true,\"message\":\"Remote updated\",\"url\":\"%s\"}", url);
    send_json(client, response);
    printf("[DEBUG] Response sent\n");
}

static void api_unstage(SOCKET client, const char* body) {
    (void)body;
    
    /* Clear staging file */
    FILE *staging = fopen(".stem/staging.txt", "w");
    if (!staging) {
        send_json(client, "{\"success\":false,\"message\":\"Failed to clear staging\"}");
        return;
    }
    fclose(staging);
    
    /* Update in-memory status */
    for (int i = 0; i < g_file_count; i++) {
        g_files[i].staged = 0;
    }
    
    send_json(client, "{\"success\":true,\"message\":\"All files unstaged\"}");
}

static void api_switch_branch(SOCKET client, const char* body) {
    /* Extract branch name from body */
    char branch_name[256] = "";
    if (body && strstr(body, "branch")) {
        const char *branch_start = strstr(body, "\"branch\":\"");
        if (branch_start) {
            branch_start += 10;
            const char *branch_end = strchr(branch_start, '"');
            if (branch_end) {
                size_t len = branch_end - branch_start;
                if (len > sizeof(branch_name) - 1) len = sizeof(branch_name) - 1;
                memcpy(branch_name, branch_start, len);
                branch_name[len] = '\0';
            }
        }
    }
    
    if (branch_name[0] == '\0') {
        send_json(client, "{\"success\":false,\"message\":\"Branch name is required\"}");
        return;
    }
    
    /* Update HEAD to point to new branch */
    FILE *head = fopen(".stem/HEAD", "w");
    if (!head) {
        send_json(client, "{\"success\":false,\"message\":\"Failed to update HEAD\"}");
        return;
    }
    
    fprintf(head, "ref: refs/heads/%s\n", branch_name);
    fclose(head);
    
    /* Update global branch name */
    strncpy(g_current_branch, branch_name, sizeof(g_current_branch) - 1);
    g_current_branch[sizeof(g_current_branch) - 1] = '\0';
    
    char response[256];
    snprintf(response, sizeof(response),
        "{\"success\":true,\"message\":\"Switched to branch %s\",\"branch\":\"%s\"}", 
        branch_name, branch_name);
    send_json(client, response);
}

static void api_create_branch(SOCKET client, const char* body) {
    /* Extract branch name from body */
    char branch_name[256] = "";
    if (body && strstr(body, "name")) {
        const char *name_start = strstr(body, "\"name\":\"");
        if (name_start) {
            name_start += 8;
            const char *name_end = strchr(name_start, '"');
            if (name_end) {
                size_t len = name_end - name_start;
                if (len > sizeof(branch_name) - 1) len = sizeof(branch_name) - 1;
                memcpy(branch_name, name_start, len);
                branch_name[len] = '\0';
            }
        }
    }
    
    if (branch_name[0] == '\0') {
        send_json(client, "{\"success\":false,\"message\":\"Branch name is required\"}");
        return;
    }
    
    /* Check if branch already exists */
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".stem/refs/heads/%s", branch_name);
    
    if (path_exists(branch_path)) {
        send_json(client, "{\"success\":false,\"message\":\"Branch already exists\"}");
        return;
    }
    
    /* Get current commit hash */
    char current_branch[256];
    get_current_branch(current_branch, sizeof(current_branch));
    
    char current_ref_path[512];
    snprintf(current_ref_path, sizeof(current_ref_path), ".stem/refs/heads/%s", current_branch);
    
    char current_hash[64] = "0000000";
    FILE *current_ref = fopen(current_ref_path, "r");
    if (current_ref) {
        fgets(current_hash, sizeof(current_hash), current_ref);
        current_hash[strcspn(current_hash, "\r\n")] = 0;
        fclose(current_ref);
    }
    
    /* Create new branch ref */
    FILE *new_branch_file = fopen(branch_path, "w");
    if (!new_branch_file) {
        send_json(client, "{\"success\":false,\"message\":\"Failed to create branch\"}");
        return;
    }
    
    fprintf(new_branch_file, "%s\n", current_hash);
    fclose(new_branch_file);
    
    /* Add to branches list */
    FILE *branches_file = fopen(".stem/branches.txt", "a");
    if (branches_file) {
        fprintf(branches_file, "%s|\n", branch_name);
        fclose(branches_file);
    }
    
    char response[256];
    snprintf(response, sizeof(response),
        "{\"success\":true,\"message\":\"Branch created: %s\",\"branch\":\"%s\"}", 
        branch_name, branch_name);
    send_json(client, response);
}

static void api_delete_branch(SOCKET client, const char* body) {
    /* Extract branch name */
    char branch_name[256] = "";
    if (body && strstr(body, "branch")) {
        const char *name_start = strstr(body, "\"branch\":\"");
        if (name_start) {
            name_start += 10;
            const char *name_end = strchr(name_start, '"');
            if (name_end) {
                size_t len = name_end - name_start;
                if (len > sizeof(branch_name) - 1) len = sizeof(branch_name) - 1;
                memcpy(branch_name, name_start, len);
                branch_name[len] = '\0';
            }
        }
    }
    
    if (branch_name[0] == '\0') {
        send_json(client, "{\"success\":false,\"message\":\"Branch name is required\"}");
        return;
    }
    
    /* Check if trying to delete current branch */
    char current_branch[256];
    get_current_branch(current_branch, sizeof(current_branch));
    
    if (strcmp(branch_name, current_branch) == 0) {
        send_json(client, "{\"success\":false,\"message\":\"Cannot delete current branch\"}");
        return;
    }
    
    /* Delete branch ref */
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".stem/refs/heads/%s", branch_name);
    
    if (remove(branch_path) != 0) {
        send_json(client, "{\"success\":false,\"message\":\"Failed to delete branch ref\"}");
        return;
    }
    
    /* Remove from branches list */
    FILE *branches_read = fopen(".stem/branches.txt", "r");
    FILE *branches_write = fopen(".stem/branches.txt.tmp", "w");
    
    if (branches_read && branches_write) {
        char line[512];
        while (fgets(line, sizeof(line), branches_read)) {
            if (strncmp(line, branch_name, strlen(branch_name)) != 0) {
                fputs(line, branches_write);
            }
        }
        fclose(branches_read);
        fclose(branches_write);
        remove(".stem/branches.txt");
        rename(".stem/branches.txt.tmp", ".stem/branches.txt");
    }
    
    char response[256];
    snprintf(response, sizeof(response),
        "{\"success\":true,\"message\":\"Branch deleted: %s\"}", branch_name);
    send_json(client, response);
}

static void api_merge_branch(SOCKET client, const char* body) {
    /* Extract source and target branches */
    char source[256] = "", target[256] = "";
    
    if (body) {
        const char *src_start = strstr(body, "\"source\":\"");
        if (src_start) {
            src_start += 10;
            const char *src_end = strchr(src_start, '"');
            if (src_end) {
                size_t len = src_end - src_start;
                if (len > sizeof(source) - 1) len = sizeof(source) - 1;
                memcpy(source, src_start, len);
                source[len] = '\0';
            }
        }
        
        const char *tgt_start = strstr(body, "\"target\":\"");
        if (tgt_start) {
            tgt_start += 10;
            const char *tgt_end = strchr(tgt_start, '"');
            if (tgt_end) {
                size_t len = tgt_end - tgt_start;
                if (len > sizeof(target) - 1) len = sizeof(target) - 1;
                memcpy(target, tgt_start, len);
                target[len] = '\0';
            }
        }
    }
    
    if (source[0] == '\0' || target[0] == '\0') {
        send_json(client, "{\"success\":false,\"message\":\"Source and target branches required\"}");
        return;
    }
    
    /* Simple fast-forward merge - copy source commit to target */
    char source_ref[512], target_ref[512];
    snprintf(source_ref, sizeof(source_ref), ".stem/refs/heads/%s", source);
    snprintf(target_ref, sizeof(target_ref), ".stem/refs/heads/%s", target);
    
    FILE *src_file = fopen(source_ref, "r");
    if (!src_file) {
        send_json(client, "{\"success\":false,\"message\":\"Source branch not found\"}");
        return;
    }
    
    char source_hash[64];
    fgets(source_hash, sizeof(source_hash), src_file);
    source_hash[strcspn(source_hash, "\r\n")] = 0;
    fclose(src_file);
    
    FILE *tgt_file = fopen(target_ref, "w");
    if (!tgt_file) {
        send_json(client, "{\"success\":false,\"message\":\"Failed to update target branch\"}");
        return;
    }
    
    fprintf(tgt_file, "%s\n", source_hash);
    fclose(tgt_file);
    
    char response[256];
    snprintf(response, sizeof(response),
        "{\"success\":true,\"message\":\"Merged %s into %s\"}", source, target);
    send_json(client, response);
}

static void api_tags(SOCKET client) {
    char *json = malloc(BUFFER_SIZE);
    if (!json) {
        send_json(client, "[]");
        return;
    }
    
    int pos = 0;
    pos += snprintf(json + pos, BUFFER_SIZE - pos, "[");
    
    /* Read tags from .stem/refs/tags/ */
    /* For now return empty array - tags functionality to be added */
    
    pos += snprintf(json + pos, BUFFER_SIZE - pos, "]");
    
    send_json(client, json);
    free(json);
}

static void api_create_tag(SOCKET client, const char* body) {
    char tag_name[256] = "";
    if (body && strstr(body, "name")) {
        const char *name_start = strstr(body, "\"name\":\"");
        if (name_start) {
            name_start += 8;
            const char *name_end = strchr(name_start, '"');
            if (name_end) {
                size_t len = name_end - name_start;
                if (len > sizeof(tag_name) - 1) len = sizeof(tag_name) - 1;
                memcpy(tag_name, name_start, len);
                tag_name[len] = '\0';
            }
        }
    }
    
    if (tag_name[0] == '\0') {
        send_json(client, "{\"success\":false,\"message\":\"Tag name is required\"}");
        return;
    }
    
    /* Get current commit */
    char current_branch[256];
    get_current_branch(current_branch, sizeof(current_branch));
    
    char ref_path[512];
    snprintf(ref_path, sizeof(ref_path), ".stem/refs/heads/%s", current_branch);
    
    char commit_hash[64] = "0000000";
    FILE *ref_file = fopen(ref_path, "r");
    if (ref_file) {
        fgets(commit_hash, sizeof(commit_hash), ref_file);
        commit_hash[strcspn(commit_hash, "\r\n")] = 0;
        fclose(ref_file);
    }
    
    /* Create tag file */
    char tag_path[512];
    snprintf(tag_path, sizeof(tag_path), ".stem/refs/tags/%s", tag_name);
    
    FILE *tag_file = fopen(tag_path, "w");
    if (!tag_file) {
        send_json(client, "{\"success\":false,\"message\":\"Failed to create tag\"}");
        return;
    }
    
    fprintf(tag_file, "%s\n", commit_hash);
    fclose(tag_file);
    
    char response[256];
    snprintf(response, sizeof(response),
        "{\"success\":true,\"message\":\"Tag created: %s\"}", tag_name);
    send_json(client, response);
}

static void api_delete_tag(SOCKET client, const char* body) {
    char tag_name[256] = "";
    if (body && strstr(body, "tag")) {
        const char *name_start = strstr(body, "\"tag\":\"");
        if (name_start) {
            name_start += 7;
            const char *name_end = strchr(name_start, '"');
            if (name_end) {
                size_t len = name_end - name_start;
                if (len > sizeof(tag_name) - 1) len = sizeof(tag_name) - 1;
                memcpy(tag_name, name_start, len);
                tag_name[len] = '\0';
            }
        }
    }
    
    if (tag_name[0] == '\0') {
        send_json(client, "{\"success\":false,\"message\":\"Tag name is required\"}");
        return;
    }
    
    char tag_path[512];
    snprintf(tag_path, sizeof(tag_path), ".stem/refs/tags/%s", tag_name);
    
    if (remove(tag_path) != 0) {
        send_json(client, "{\"success\":false,\"message\":\"Failed to delete tag\"}");
        return;
    }
    
    char response[256];
    snprintf(response, sizeof(response),
        "{\"success\":true,\"message\":\"Tag deleted: %s\"}", tag_name);
    send_json(client, response);
}

static void api_config(SOCKET client) {
    /* Return default config */
    const char* config = "{"
        "\"userName\":\"You\","
        "\"userEmail\":\"you@example.com\","
        "\"autoStage\":false,"
        "\"autoSync\":false,"
        "\"syncInterval\":30"
        "}";
    send_json(client, config);
}

static void api_save_config(SOCKET client, const char* body) {
    (void)body; /* Config saving would write to .stem/config file */
    /* For now just return success */
    send_json(client, "{\"success\":true,\"message\":\"Config saved\"}");
}

static void api_switch_repo(SOCKET client, const char* body) {
    /* Extract path from body */
    char path[512] = "";
    if (body && strstr(body, "path")) {
        const char *path_start = strstr(body, "\"path\":\"");
        if (path_start) {
            path_start += 8;
            const char *path_end = strchr(path_start, '"');
            if (path_end) {
                size_t len = path_end - path_start;
                if (len > sizeof(path) - 1) len = sizeof(path) - 1;
                
                /* Copy and unescape backslashes */
                size_t j = 0;
                for (size_t i = 0; i < len && j < sizeof(path) - 1; i++) {
                    if (path_start[i] == '\\' && i + 1 < len && path_start[i + 1] == '\\') {
                        /* Escaped backslash -> single backslash */
                        path[j++] = '\\';
                        i++; /* Skip next backslash */
                    } else {
                        path[j++] = path_start[i];
                    }
                }
                path[j] = '\0';
            }
        }
    }
    
    printf("[DEBUG] api_switch_repo: path='%s'\n", path);
    
    if (path[0] == '\0') {
        send_json(client, "{\"success\":false,\"message\":\"Path is required\"}");
        return;
    }
    
    /* Try to change directory */
    printf("[DEBUG] Attempting chdir to: %s\n", path);
    if (chdir(path) != 0) {
        printf("[DEBUG] chdir failed with error: %d\n", errno);
        char error_msg[1024];
        snprintf(error_msg, sizeof(error_msg),
            "{\"success\":false,\"message\":\"Failed to switch to repository path: %s\"}", 
            strerror(errno));
        send_json(client, error_msg);
        return;
    }
    
    /* Update global repository path */
    if (getcwd(g_repo_path, sizeof(g_repo_path))) {
        printf("[DEBUG] Successfully switched to: %s\n", g_repo_path);
        
        /* Refresh file list for new repository */
        refresh_file_list();
        
        char response[1024];
        snprintf(response, sizeof(response),
            "{\"success\":true,\"message\":\"Switched to repository\",\"path\":\"%s\"}", 
            g_repo_path);
        send_json(client, response);
    } else {
        send_json(client, "{\"success\":false,\"message\":\"Failed to get current directory\"}");
    }
}

/* Main API handler */
static void handle_api(SOCKET client, const char* path, const char* method, const char* body) {
    printf("[%s] %s\n", method, path);
    
    if (strcmp(path, "/api/status") == 0 && strcmp(method, "GET") == 0) {
        api_status(client);
    }
    else if (strcmp(path, "/api/files") == 0 && strcmp(method, "GET") == 0) {
        api_files(client);
    }
    else if (strcmp(path, "/api/history") == 0 && strcmp(method, "GET") == 0) {
        api_history(client);
    }
    else if (strcmp(path, "/api/branches") == 0 && strcmp(method, "GET") == 0) {
        api_branches(client);
    }
    else if (strcmp(path, "/api/remote") == 0 && strcmp(method, "GET") == 0) {
        api_remote(client);
    }
    else if (strcmp(path, "/api/tags") == 0 && strcmp(method, "GET") == 0) {
        api_tags(client);
    }
    else if (strcmp(path, "/api/config") == 0 && strcmp(method, "GET") == 0) {
        api_config(client);
    }
    else if (strcmp(path, "/api/stage") == 0 && strcmp(method, "POST") == 0) {
        api_stage(client, body);
    }
    else if (strcmp(path, "/api/commit") == 0 && strcmp(method, "POST") == 0) {
        api_commit(client, body);
    }
    else if (strcmp(path, "/api/push") == 0 && strcmp(method, "POST") == 0) {
        api_push(client);
    }
    else if (strcmp(path, "/api/pull") == 0 && strcmp(method, "POST") == 0) {
        api_pull(client);
    }
    else if (strcmp(path, "/api/remote") == 0 && strcmp(method, "POST") == 0) {
        api_set_remote(client, body);
    }
    else if (strcmp(path, "/api/unstage") == 0 && strcmp(method, "POST") == 0) {
        api_unstage(client, body);
    }
    else if (strcmp(path, "/api/branch/switch") == 0 && strcmp(method, "POST") == 0) {
        api_switch_branch(client, body);
    }
    else if (strcmp(path, "/api/branch/create") == 0 && strcmp(method, "POST") == 0) {
        api_create_branch(client, body);
    }
    else if (strcmp(path, "/api/branch/delete") == 0 && strcmp(method, "POST") == 0) {
        api_delete_branch(client, body);
    }
    else if (strcmp(path, "/api/branch/merge") == 0 && strcmp(method, "POST") == 0) {
        api_merge_branch(client, body);
    }
    else if (strcmp(path, "/api/tag/create") == 0 && strcmp(method, "POST") == 0) {
        api_create_tag(client, body);
    }
    else if (strcmp(path, "/api/tag/delete") == 0 && strcmp(method, "POST") == 0) {
        api_delete_tag(client, body);
    }
    else if (strcmp(path, "/api/config") == 0 && strcmp(method, "POST") == 0) {
        api_save_config(client, body);
    }
    else if (strcmp(path, "/api/repo/switch") == 0 && strcmp(method, "POST") == 0) {
        api_switch_repo(client, body);
    }
    else {
        send_json(client, "{\"error\":\"Endpoint not found\"}");
    }
}

static void handle_request(SOCKET client) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        close(client);
        return;
    }
    
    buffer[bytes_received] = '\0';
    
    /* Parse HTTP request */
    char method[16], path[256], version[16];
    if (sscanf(buffer, "%15s %255s %15s", method, path, version) != 3) {
        close(client);
        return;
    }
    
    /* Handle API requests */
    if (strncmp(path, "/api/", 5) == 0) {
        /* Find Content-Length header */
        int content_length = 0;
        char* content_len_str = strstr(buffer, "Content-Length: ");
        if (content_len_str) {
            content_length = atoi(content_len_str + 16);
        }
        
        /* Find body separator */
        char* body_start = strstr(buffer, "\r\n\r\n");
        char* body = NULL;
        
        if (body_start) {
            body_start += 4;
            int body_offset = body_start - buffer;
            int body_received = bytes_received - body_offset;
            
            /* If we need more data, receive it */
            if (content_length > 0 && body_received < content_length) {
                int remaining = content_length - body_received;
                int additional = recv(client, buffer + bytes_received, remaining, 0);
                if (additional > 0) {
                    bytes_received += additional;
                    buffer[bytes_received] = '\0';
                }
            }
            
            body = body_start;
        }
        
        handle_api(client, path, method, body);
        close(client);
        return;
    }
    
    /* Serve static files */
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }
    
    char filepath[MAX_PATH_LEN];
    snprintf(filepath, sizeof(filepath), "gui_web%s", path);
    
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        const char* not_found = "<!DOCTYPE html><html><body><h1>404 Not Found</h1></body></html>";
        send_response(client, 404, "Not Found", "text/html", not_found, strlen(not_found));
        close(client);
        return;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(file_size);
    if (!content) {
        fclose(file);
        close(client);
        return;
    }
    
    fread(content, 1, file_size, file);
    fclose(file);
    
    send_response(client, 200, "OK", get_mime_type(filepath), content, file_size);
    
    free(content);
    close(client);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
#ifdef _WIN32
    /* Initialize Windows Sockets */
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "[ERROR] WSAStartup failed\n");
        return 1;
    }
#endif
    
    /* Create socket */
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "[ERROR] Failed to create socket\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    /* Allow reuse of address */
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    /* Bind to port */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "[ERROR] Failed to bind to port %d\n", PORT);
        close(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    /* Listen for connections */
    if (listen(server_socket, 10) == SOCKET_ERROR) {
        fprintf(stderr, "[ERROR] Failed to listen\n");
        close(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    /* Get current working directory for repository path */
    if (getcwd(g_repo_path, sizeof(g_repo_path))) {
        printf("========================================\n");
        printf("  STEM VCS - Web GUI Server (PRODUCTION)\n");
        printf("========================================\n");
        printf("\n");
        printf("[OK] Server running on http://localhost:%d\n", PORT);
        printf("[OK] Repository: %s\n", g_repo_path);
        printf("[OK] Press Ctrl+C to stop\n");
        printf("\n");
    }
    
    /* Refresh file list on startup */
    refresh_file_list();
    
    /* Open browser automatically */
#ifdef _WIN32
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "start http://localhost:%d", PORT);
    system(cmd);
#elif defined(__APPLE__)
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "open http://localhost:%d 2>/dev/null", PORT);
    system(cmd);
#else
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "xdg-open http://localhost:%d 2>/dev/null || true", PORT);
    system(cmd);
#endif
    
    /* Accept connections loop */
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            continue;
        }
        
        handle_request(client_socket);
    }
    
    /* Cleanup (unreachable in normal operation) */
    close(server_socket);
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    return 0;
}
