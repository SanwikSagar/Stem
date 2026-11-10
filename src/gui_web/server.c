/* STEM VCS - Web GUI Server
 * Cross-platform local web server for STEM GUI
 * Runs on Windows, macOS, and Linux
 */

#include "stem/stem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "ws2_32.lib")
    #else
        /* MinGW: Link with -lws2_32 */
    #endif
    #define close closesocket
    typedef int socklen_t;
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#define PORT 8765
#define BUFFER_SIZE 65536
#define MAX_PATH_LEN 1024

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

static void handle_api(SOCKET client, const char* path, const char* method, const char* body) {
    (void)body;  /* Unused for now - will be used when parsing POST data */
    
    /* API: Get repository status */
    if (strcmp(path, "/api/status") == 0 && strcmp(method, "GET") == 0) {
        const char* json = "{\"initialized\":true,\"branch\":\"main\",\"status\":\"clean\"}";
        send_response(client, 200, "OK", "application/json", json, strlen(json));
        return;
    }
    
    /* API: Get file list */
    if (strcmp(path, "/api/files") == 0 && strcmp(method, "GET") == 0) {
        const char* json = "["
            "{\"path\":\"src/renderer/Renderer.cpp\",\"status\":\"modified\",\"staged\":true,\"additions\":102,\"deletions\":18},"
            "{\"path\":\"src/renderer/Shader.cpp\",\"status\":\"modified\",\"staged\":true,\"additions\":28,\"deletions\":6},"
            "{\"path\":\"include/Renderer.h\",\"status\":\"modified\",\"staged\":true,\"additions\":12,\"deletions\":3},"
            "{\"path\":\"README.md\",\"status\":\"modified\",\"staged\":false,\"additions\":5,\"deletions\":2},"
            "{\"path\":\"CMakeLists.txt\",\"status\":\"modified\",\"staged\":false,\"additions\":3,\"deletions\":1}"
            "]";
        send_response(client, 200, "OK", "application/json", json, strlen(json));
        return;
    }
    
    /* API: Get commit history */
    if (strcmp(path, "/api/history") == 0 && strcmp(method, "GET") == 0) {
        const char* json = "["
            "{\"hash\":\"a1b2c3d\",\"message\":\"Improve renderer performance\",\"author\":\"You\",\"date\":\"2 hours ago\",\"files\":3,\"additions\":142,\"deletions\":27},"
            "{\"hash\":\"d4e5f6g\",\"message\":\"Refactor memory allocator\",\"author\":\"You\",\"date\":\"1 day ago\",\"files\":5,\"additions\":85,\"deletions\":42},"
            "{\"hash\":\"h7i8j9k\",\"message\":\"Add Vulkan backend\",\"author\":\"Alex\",\"date\":\"2 days ago\",\"files\":12,\"additions\":320,\"deletions\":15},"
            "{\"hash\":\"l1m2n3o\",\"message\":\"Fix asset import crash\",\"author\":\"You\",\"date\":\"3 days ago\",\"files\":2,\"additions\":15,\"deletions\":8},"
            "{\"hash\":\"p4q5r6s\",\"message\":\"Improve logging system\",\"author\":\"Sam\",\"date\":\"5 days ago\",\"files\":4,\"additions\":95,\"deletions\":30},"
            "{\"hash\":\"t7u8v9w\",\"message\":\"Initial commit\",\"author\":\"You\",\"date\":\"1 week ago\",\"files\":45,\"additions\":2500,\"deletions\":0}"
            "]";
        send_response(client, 200, "OK", "application/json", json, strlen(json));
        return;
    }
    
    /* API: Get branches */
    if (strcmp(path, "/api/branches") == 0 && strcmp(method, "GET") == 0) {
        const char* json = "["
            "{\"name\":\"main\",\"current\":true,\"ahead\":2,\"behind\":0},"
            "{\"name\":\"feature-vulkan\",\"current\":false,\"ahead\":5,\"behind\":1},"
            "{\"name\":\"bugfix-memory\",\"current\":false,\"ahead\":0,\"behind\":3}"
            "]";
        send_response(client, 200, "OK", "application/json", json, strlen(json));
        return;
    }
    
    /* API: Get remote */
    if (strcmp(path, "/api/remote") == 0 && strcmp(method, "GET") == 0) {
        const char* json = "{\"name\":\"Google Drive\",\"url\":\"https://drive.google.com/drive/folders/1a2b3c4d5e6f\",\"connected\":true,\"size\":\"12.4 GB\",\"total\":\"100 GB\",\"percent\":12}";
        send_response(client, 200, "OK", "application/json", json, strlen(json));
        return;
    }
    
    /* API: Stage file */
    if (strcmp(path, "/api/stage") == 0 && strcmp(method, "POST") == 0) {
        const char* json = "{\"success\":true,\"message\":\"File staged\"}";
        send_response(client, 200, "OK", "application/json", json, strlen(json));
        return;
    }
    
    /* API: Create commit */
    if (strcmp(path, "/api/commit") == 0 && strcmp(method, "POST") == 0) {
        const char* json = "{\"success\":true,\"message\":\"Commit created\",\"hash\":\"xyz789\"}";
        send_response(client, 200, "OK", "application/json", json, strlen(json));
        return;
    }
    
    /* API: Push to cloud */
    if (strcmp(path, "/api/push") == 0 && strcmp(method, "POST") == 0) {
        const char* json = "{\"success\":true,\"message\":\"Pushed to cloud\"}";
        send_response(client, 200, "OK", "application/json", json, strlen(json));
        return;
    }
    
    /* API: Pull from cloud */
    if (strcmp(path, "/api/pull") == 0 && strcmp(method, "POST") == 0) {
        const char* json = "{\"success\":true,\"message\":\"Pulled from cloud\"}";
        send_response(client, 200, "OK", "application/json", json, strlen(json));
        return;
    }
    
    /* 404 Not Found */
    const char* error = "{\"error\":\"API endpoint not found\"}";
    send_response(client, 404, "Not Found", "application/json", error, strlen(error));
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
    
    printf("[%s] %s\n", method, path);
    
    /* Handle API requests */
    if (strncmp(path, "/api/", 5) == 0) {
        /* Find body (after \r\n\r\n) */
        char* body = strstr(buffer, "\r\n\r\n");
        if (body) body += 4;
        
        handle_api(client, path, method, body);
        close(client);
        return;
    }
    
    /* Serve static files */
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }
    
    /* Build file path */
    char filepath[MAX_PATH_LEN];
    snprintf(filepath, sizeof(filepath), "gui_web%s", path);
    
    /* Read file */
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
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
#endif
    
    /* Create socket */
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\n");
        return 1;
    }
    
    /* Allow reuse of address */
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    /* Bind to port */
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Failed to bind to port %d\n", PORT);
        close(server_socket);
        return 1;
    }
    
    /* Listen for connections */
    if (listen(server_socket, 10) == SOCKET_ERROR) {
        fprintf(stderr, "Failed to listen\n");
        close(server_socket);
        return 1;
    }
    
    printf("========================================\n");
    printf("  STEM VCS - Web GUI Server\n");
    printf("========================================\n");
    printf("\n");
    printf("[OK] Server running on http://localhost:%d\n", PORT);
    printf("[OK] Press Ctrl+C to stop\n");
    printf("\n");
    
    /* Open browser automatically */
#ifdef _WIN32
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "start http://localhost:%d", PORT);
    system(cmd);
#elif defined(__APPLE__)
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "open http://localhost:%d", PORT);
    system(cmd);
#else
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "xdg-open http://localhost:%d 2>/dev/null", PORT);
    system(cmd);
#endif
    
    /* Accept connections */
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            continue;
        }
        
        handle_request(client_socket);
    }
    
    close(server_socket);
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    return 0;
}
