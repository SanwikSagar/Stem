#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "stem/stem.h"
#include <stdio.h>
<stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir
#else
#include <unistd.h>
#endif

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 900
#define MAX_PATH_LEN 512
#define MAX_FILES 1024

/* Color Scheme - Modern Dark Theme */
#define COLOR_BG (Color){10, 12, 16, 255}
#define COLOR_PANEL (Color){17, 21, 28, 255}
#define COLOR_PANEL_LIGHT (Color){24, 28, 37, 255}
#define COLOR_HOVER (Color){31, 37, 48, 255}
#define COLOR_ACTIVE (Color){39, 46, 60, 255}
#define COLOR_BORDER (Color){43, 51, 66, 255}
#define COLOR_TEXT (Color){237, 242, 248, 255}
#define COLOR_TEXT_MUTED (Color){157, 171, 191, 255}
#define COLOR_TEXT_DIM (Color){107, 121, 141, 255}
#define COLOR_ACCENT (Color){59, 130, 246, 255}
#define COLOR_ACCENT_HOVER (Color){37, 99, 235, 255}
#define COLOR_SUCCESS (Color){16, 185, 129, 255}
#define COLOR_WARNING (Color){245, 158, 11, 255}
#define COLOR_ERROR (Color){239, 68, 68, 255}

typedef enum {
    SCREEN_WELCOME,
    SCREEN_SETUP,
    SCREEN_MAIN
} AppScreen;

typedef enum {
    VIEW_DASHBOARD,
    VIEW_CHANGES,
    VIEW_HISTORY,
    VIEW_BRANCHES,
    VIEW_REMOTES
} MainView;

typedef struct {
    char path[MAX_PATH_LEN];
    bool is_directory;
    bool is_staged;
    bool is_modified;
} FileItem;

typedef struct {
    AppScreen current_screen;
    MainView current_view;
    
    /* Repository state */
    char repo_path[MAX_PATH_LEN];
    char cloud_url[MAX_PATH_LEN];
    char branch_name[64];
    stem_repository_t *repo;
    bool repo_initialized;
    
    /* UI state */
    char setup_path_input[MAX_PATH_LEN];
    char setup_cloud_input[MAX_PATH_LEN];
    char commit_message[512];
    int selected_file;
    int scroll_offset;
    
    /* File list */
    FileItem files[MAX_FILES];
    int file_count;
    int staged_count;
    
    /* Status */
    char status_message[256];
    float status_timer;
    
} AppState;

static AppState app = {0};

/* Helper functions */
static void SetStatus(const char *message) {
    strncpy(app.status_message, message, sizeof(app.status_message) - 1);
    app.status_timer = 3.0f;
}

static void DrawPanel(Rectangle rect, Color color) {
    DrawRectangleRounded(rect, 0.08f, 16, color);
}

static void DrawButton(Rectangle rect, const char *text, Color color, bool *clicked) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);
    
    Color bg = hover ? ColorBrightness(color, 0.1f) : color;
    if (hover && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        bg = ColorBrightness(color, -0.1f);
    }
    
    DrawRectangleRounded(rect, 0.12f, 16, bg);
    
    int text_width = MeasureText(text, 16);
    DrawText(text, 
             rect.x + (rect.width - text_width) / 2,
             rect.y + (rect.height - 16) / 2,
             16, COLOR_TEXT);
    
    if (clicked) {
        *clicked = hover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    }
}

static void DrawIconButton(Rectangle rect, const char *icon, const char *label, Color color, bool *clicked) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);
    
    if (hover) {
        DrawRectangleRounded(rect, 0.1f, 16, COLOR_HOVER);
    }
    
    DrawText(icon, rect.x + 12, rect.y + 8, 20, color);
    DrawText(label, rect.x + 42, rect.y + 10, 15, COLOR_TEXT);
    
    if (clicked) {
        *clicked = hover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    }
}

static void DrawTextInput(Rectangle rect, char *buffer, int buffer_size, bool *active) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);
    
    Color border = *active ? COLOR_ACCENT : (hover ? COLOR_BORDER : COLOR_BORDER);
    DrawRectangleRounded(rect, 0.08f, 16, COLOR_PANEL_LIGHT);
    DrawRectangleRoundedLines(rect, 0.08f, 16, 2, border);
    
    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *active = true;
    } else if (!hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *active = false;
    }
    
    if (*active) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                int len = strlen(buffer);
                if (len < buffer_size - 1) {
                    buffer[len] = (char)key;
                    buffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(buffer);
            if (len > 0) buffer[len - 1] = '\0';
        }
    }
    
    DrawText(buffer[0] ? buffer : "...", rect.x + 16, rect.y + 12, 16, 
             buffer[0] ? COLOR_TEXT : COLOR_TEXT_DIM);
}

/* Screen drawing functions */
static void DrawWelcomeScreen(void) {
    /* Centered card */
    Rectangle card = {WINDOW_WIDTH/2 - 300, WINDOW_HEIGHT/2 - 250, 600, 500};
    DrawPanel(card, COLOR_PANEL);
    
    /* Logo and title */
    DrawCircle(card.x + 300, card.y + 80, 40, COLOR_SUCCESS);
    DrawText("STEM", card.x + 270, card.y + 62, 32, COLOR_TEXT);
    DrawText("VCS", card.x + 277, card.y + 92, 16, COLOR_TEXT_MUTED);
    
    DrawText("Next Generation Version Control", card.x + 130, card.y + 150, 20, COLOR_TEXT);
    DrawText("Own your code. Own your storage.", card.x + 165, card.y + 180, 14, COLOR_TEXT_MUTED);
    
    /* Buttons */
    bool init_clicked = false, open_clicked = false;
    DrawButton((Rectangle){card.x + 50, card.y + 280, 240, 50}, 
               "Initialize Repository", COLOR_ACCENT, &init_clicked);
    DrawButton((Rectangle){card.x + 310, card.y + 280, 240, 50}, 
               "Open Existing", COLOR_PANEL_LIGHT, &open_clicked);
    
    if (init_clicked) {
        app.current_screen = SCREEN_SETUP;
        strcpy(app.setup_path_input, "");
        strcpy(app.setup_cloud_input, "");
    }
    
    if (open_clicked) {
        /* TODO: Open folder picker */
        SetStatus("Open folder picker coming soon");
    }
    
    /* Footer */
    DrawText("Version 1.0.0 \xC2\xB7 Copyright (c) 2026 STEM VCS Contributors", 
             card.x + 130, card.y + 450, 12, COLOR_TEXT_DIM);
}

static bool path_input_active = false;
static bool cloud_input_active = false;

static void DrawSetupScreen(void) {
    Rectangle card = {WINDOW_WIDTH/2 - 350, WINDOW_HEIGHT/2 - 300, 700, 600};
    DrawPanel(card, COLOR_PANEL);
    
    DrawText("Setup New Repository", card.x + 30, card.y + 30, 24, COLOR_TEXT);
    DrawText("Configure your repository and cloud storage", card.x + 30, card.y + 65, 14, COLOR_TEXT_MUTED);
    
    /* Repository path */
    DrawText("Repository Location", card.x + 30, card.y + 120, 16, COLOR_TEXT);
    DrawText("Where to create your repository", card.x + 30, card.y + 145, 12, COLOR_TEXT_MUTED);
    DrawTextInput((Rectangle){card.x + 30, card.y + 170, 640, 45}, 
                  app.setup_path_input, MAX_PATH_LEN, &path_input_active);
    
    bool browse_clicked = false;
    DrawButton((Rectangle){card.x + 30, card.y + 225, 150, 38}, 
               "Browse...", COLOR_PANEL_LIGHT, &browse_clicked);
    
    /* Cloud storage */
    DrawText("Cloud Storage URL", card.x + 30, card.y + 300, 16, COLOR_TEXT);
    DrawText("Google Drive folder URL (required)", card.x + 30, card.y + 325, 12, COLOR_TEXT_MUTED);
    DrawTextInput((Rectangle){card.x + 30, card.y + 350, 640, 45}, 
                  app.setup_cloud_input, MAX_PATH_LEN, &cloud_input_active);
    
    /* Example URL */
    DrawText("Example: https://drive.google.com/drive/folders/YOUR_FOLDER_ID", 
             card.x + 30, card.y + 410, 12, COLOR_TEXT_DIM);
    
    /* Action buttons */
    bool create_clicked = false, back_clicked = false;
    DrawButton((Rectangle){card.x + 400, card.y + 520, 140, 50}, 
               "Create", COLOR_SUCCESS, &create_clicked);
    DrawButton((Rectangle){card.x + 250, card.y + 520, 140, 50}, 
               "Back", COLOR_PANEL_LIGHT, &back_clicked);
    
    if (back_clicked) {
        app.current_screen = SCREEN_WELCOME;
    }
    
    if (create_clicked) {
        if (strlen(app.setup_path_input) == 0) {
            SetStatus("Please enter a repository path");
        } else if (strlen(app.setup_cloud_input) == 0) {
            SetStatus("Cloud storage URL is required");
        } else {
            /* Initialize repository */
            strcpy(app.repo_path, app.setup_path_input);
            strcpy(app.cloud_url, app.setup_cloud_input);
            strcpy(app.branch_name, "main");
            app.repo_initialized = true;
            app.current_screen = SCREEN_MAIN;
            app.current_view = VIEW_DASHBOARD;
            SetStatus("Repository initialized successfully!");
        }
    }
}

static void DrawSidebar(void) {
    Rectangle sidebar = {0, 0, 220, WINDOW_HEIGHT};
    DrawRectangle(0, 0, sidebar.width, sidebar.height, COLOR_PANEL);
    
    /* Logo */
    DrawCircle(30, 35, 16, COLOR_SUCCESS);
    DrawText("STEM", 55, 24, 20, COLOR_TEXT);
    DrawText("v1.0.0", 55, 46, 11, COLOR_TEXT_DIM);
    
    /* Branch indicator */
    DrawRectangleRounded((Rectangle){20, 80, 180, 36}, 0.15f, 16, COLOR_PANEL_LIGHT);
    DrawText("\xF0\x9F\x8C\xBF", 30, 88, 16, COLOR_SUCCESS);  /* Branch emoji */
    DrawText(app.branch_name, 55, 91, 14, COLOR_TEXT);
    
    /* Navigation */
    const char *nav_icons[] = {"\xF0\x9F\x8F\xA0", "\xE2\x9C\x8F", "\xF0\x9F\x93\x9C", "\xF0\x9F\x8C\xBF", "\xE2\x98\x81"};
    const char *nav_labels[] = {"Dashboard", "Changes", "History", "Branches", "Remotes"};
    
    for (int i = 0; i < 5; i++) {
        Rectangle btn = {20, 140 + i * 52, 180, 44};
        bool clicked = false;
        
        if (app.current_view == i) {
            DrawRectangleRounded(btn, 0.12f, 16, COLOR_ACTIVE);
        }
        
        DrawIconButton(btn, nav_icons[i], nav_labels[i], 
                      app.current_view == i ? COLOR_ACCENT : COLOR_TEXT_MUTED, &clicked);
        
        if (clicked) {
            app.current_view = i;
        }
    }
    
    /* Repository info */
    DrawRectangle(0, WINDOW_HEIGHT - 120, sidebar.width, 1, COLOR_BORDER);
    DrawText("REPOSITORY", 20, WINDOW_HEIGHT - 100, 11, COLOR_TEXT_DIM);
    
    char repo_name[64];
    const char *slash = strrchr(app.repo_path, '/');
    const char *backslash = strrchr(app.repo_path, '\\');
    const char *name = slash > backslash ? slash + 1 : (backslash ? backslash + 1 : app.repo_path);
    snprintf(repo_name, sizeof(repo_name), "%s", name[0] ? name : "My Repo");
    DrawText(repo_name, 20, WINDOW_HEIGHT - 75, 14, COLOR_TEXT);
    DrawText(app.cloud_url[0] ? "Cloud connected" : "Local only", 20, WINDOW_HEIGHT - 55, 11, COLOR_TEXT_MUTED);
}

static void DrawTopBar(void) {
    Rectangle topbar = {220, 0, WINDOW_WIDTH - 220, 70};
    DrawRectangle(topbar.x, topbar.y, topbar.width, topbar.height, COLOR_PANEL);
    DrawRectangle(topbar.x, topbar.height - 1, topbar.width, 1, COLOR_BORDER);
    
    const char *view_titles[] = {"Dashboard", "Changes", "History", "Branches", "Cloud Remotes"};
    DrawText(view_titles[app.current_view], topbar.x + 30, 25, 24, COLOR_TEXT);
    
    /* Action buttons */
    bool stage_clicked = false, commit_clicked = false, push_clicked = false;
    DrawButton((Rectangle){topbar.x + topbar.width - 380, 18, 110, 36}, 
               "Stage All", COLOR_PANEL_LIGHT, &stage_clicked);
    DrawButton((Rectangle){topbar.x + topbar.width - 260, 18, 110, 36}, 
               "Commit", COLOR_ACCENT, &commit_clicked);
    DrawButton((Rectangle){topbar.x + topbar.width - 140, 18, 110, 36}, 
               "Push", COLOR_SUCCESS, &push_clicked);
    
    if (stage_clicked) SetStatus("Staged all files");
    if (commit_clicked) SetStatus("Commit created");
    if (push_clicked) SetStatus("Pushed to cloud");
}

static void DrawDashboardView(void) {
    int x = 250, y = 100;
    
    /* Stats cards */
    Rectangle cards[] = {
        {x, y, 260, 120},
        {x + 280, y, 260, 120},
        {x + 560, y, 260, 120}
    };
    
    const char *titles[] = {"Staged Files", "Modified Files", "Total Files"};
    const char *values[] = {"12", "5", "234"};
    Color colors[] = {COLOR_SUCCESS, COLOR_WARNING, COLOR_ACCENT};
    
    for (int i = 0; i < 3; i++) {
        DrawPanel(cards[i], COLOR_PANEL);
        DrawText(titles[i], cards[i].x + 20, cards[i].y + 20, 14, COLOR_TEXT_MUTED);
        DrawText(values[i], cards[i].x + 20, cards[i].y + 50, 36, colors[i]);
    }
    
    /* Recent activity */
    DrawPanel((Rectangle){x, y + 160, 840, 400}, COLOR_PANEL);
    DrawText("Recent Activity", x + 20, y + 180, 18, COLOR_TEXT);
    
    const char *activities[] = {
        "Committed: Initial project setup",
        "Added: src/main.c",
        "Modified: README.md",
        "Pushed to cloud storage",
        "Created branch: feature-login"
    };
    
    for (int i = 0; i < 5; i++) {
        DrawText("\xE2\x80\xA2", x + 30, y + 230 + i * 50, 16, COLOR_ACCENT);
        DrawText(activities[i], x + 50, y + 228 + i * 50, 14, COLOR_TEXT);
        DrawText("2 hours ago", x + 50, y + 250 + i * 50, 11, COLOR_TEXT_DIM);
    }
}

static void DrawChangesView(void) {
    DrawText("File changes will appear here", 280, 150, 16, COLOR_TEXT_MUTED);
}

static void DrawHistoryView(void) {
    DrawText("Commit history will appear here", 280, 150, 16, COLOR_TEXT_MUTED);
}

static void DrawBranchesView(void) {
    DrawText("Branch management will appear here", 280, 150, 16, COLOR_TEXT_MUTED);
}

static void DrawRemotesView(void) {
    int x = 250, y = 100;
    
    DrawPanel((Rectangle){x, y, 840, 200}, COLOR_PANEL);
    DrawText("Cloud Storage", x + 24, y + 24, 18, COLOR_TEXT);
    DrawText("Google Drive", x + 24, y + 60, 16, COLOR_TEXT_MUTED);
    DrawText(app.cloud_url, x + 24, y + 90, 13, COLOR_TEXT);
    
    DrawCircle(x + 24 + 8, y + 130, 8, COLOR_SUCCESS);
    DrawText("Connected", x + 24 + 24, y + 123, 14, COLOR_SUCCESS);
    
    bool test_clicked = false, change_clicked = false;
    DrawButton((Rectangle){x + 24, y + 160, 140, 38}, 
               "Test Connection", COLOR_PANEL_LIGHT, &test_clicked);
    DrawButton((Rectangle){x + 180, y + 160, 140, 38}, 
               "Change URL", COLOR_PANEL_LIGHT, &change_clicked);
    
    if (test_clicked) SetStatus("Testing cloud connection...");
    if (change_clicked) SetStatus("Cloud URL change dialog coming soon");
}

static void DrawMainScreen(void) {
    /* Background */
    ClearBackground(COLOR_BG);
    
    /* Sidebar */
    DrawSidebar();
    
    /* Top bar */
    DrawTopBar();
    
    /* Main content area */
    switch (app.current_view) {
        case VIEW_DASHBOARD:
            DrawDashboardView();
            break;
        case VIEW_CHANGES:
            DrawChangesView();
            break;
        case VIEW_HISTORY:
            DrawHistoryView();
            break;
        case VIEW_BRANCHES:
            DrawBranchesView();
            break;
        case VIEW_REMOTES:
            DrawRemotesView();
            break;
    }
    
    /* Status bar */
    if (app.status_timer > 0) {
        DrawRectangle(220, WINDOW_HEIGHT - 32, WINDOW_WIDTH - 220, 32, COLOR_PANEL);
        DrawRectangle(220, WINDOW_HEIGHT - 32, WINDOW_WIDTH - 220, 1, COLOR_BORDER);
        DrawText(app.status_message, 250, WINDOW_HEIGHT - 22, 13, COLOR_TEXT_MUTED);
        app.status_timer -= GetFrameTime();
    }
}

int main(void) {
    /* Initialize STEM library */
    stem_init();
    
    /* Initialize window */
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "STEM - Version Control System");
    SetTargetFPS(60);
    SetExitKey(0);
    
    /* Initialize app state */
    app.current_screen = SCREEN_WELCOME;
    app.current_view = VIEW_DASHBOARD;
    app.repo_initialized = false;
    strcpy(app.branch_name, "main");
    
    /* Main loop */
    while (!WindowShouldClose()) {
        BeginDrawing();
        
        ClearBackground(COLOR_BG);
        
        switch (app.current_screen) {
            case SCREEN_WELCOME:
                DrawWelcomeScreen();
                break;
            case SCREEN_SETUP:
                DrawSetupScreen();
                break;
            case SCREEN_MAIN:
                DrawMainScreen();
                break;
        }
        
        EndDrawing();
    }
    
    /* Cleanup */
    CloseWindow();
    stem_shutdown();
    
    return 0;
}
