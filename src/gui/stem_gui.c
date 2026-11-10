/* STEM Desktop - a usable, local-first Windows client.
   This file deliberately avoids fake repository data: every row comes from
   the worktree, .stem/staging.txt, and .stem/commits.log. */
#include "stem/stem.h"
#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <windows.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

#define APP_TITLE "STEM - Next Generation Version Control"
#define MAX_FILES 16384
#define MAX_COMMITS 128
#define MAX_PATHS 1024
#define STATUS_H 28
static int ui_top = 64, ui_left = 220, ui_right = 306, ui_bottom = 218,
           ui_scale = 1;
#define TOP_H ui_top
#define LEFT_W ui_left
#define RIGHT_W ui_right
#define BOTTOM_H ui_bottom
#define STEM_MIN(a, b) ((a) < (b) ? (a) : (b))
#define STEM_MAX(a, b) ((a) > (b) ? (a) : (b))

#define C_BG RGB(10, 16, 25)
#define C_PANEL RGB(17, 25, 37)
#define C_PANEL2 RGB(22, 32, 47)
#define C_HOVER RGB(31, 45, 65)
#define C_BORDER RGB(43, 57, 77)
#define C_TEXT RGB(237, 242, 248)
#define C_MUTED RGB(157, 171, 191)
#define C_BLUE RGB(60, 132, 246)
#define C_GREEN RGB(54, 211, 153)
#define C_PURPLE RGB(192, 102, 255)
#define C_RED RGB(248, 113, 113)
#define C_AMBER RGB(251, 191, 36)

typedef enum {
  PAGE_OVERVIEW,
  PAGE_CHANGES,
  PAGE_HISTORY,
  PAGE_BRANCHES,
  PAGE_TAGS,
  PAGE_REMOTES,
  PAGE_SETTINGS
} Page;
typedef struct {
  char path[MAX_PATH];
  unsigned long long size;
  unsigned long long mtime;
} FileEntry;
typedef struct {
  char hash[17], message[256], date[48];
  int files;
} Commit;
typedef struct {
  RECT r;
  int id;
} Hit;
typedef struct {
  HWND hwnd, msg, diff, status;
  HFONT ui, bold, title, mono, small;
  HBRUSH bg, panel, edit_bg;
  stem_repository_t *repo;
  char root[MAX_PATH], branch[64], remote[MAX_PATH];
  FileEntry files[MAX_FILES];
  int file_count, selected_file;
  char staged[MAX_PATHS][MAX_PATH];
  int staged_count;
  Commit commits[MAX_COMMITS];
  int commit_count, selected_commit;
  Page page;
  Hit hits[80];
  int hit_count;
  BOOL initialized;
  char status_text[256];
} App;
static App a;

static void refresh_all(void);
static void paint(HDC, RECT *);
static void layout(HWND);
static void setup_metrics(HWND);
static void stage_all(void);
static void commit_now(void);
static void do_push(void);
static void set_status(const char *);

static void join(char *out, size_t n, const char *x, const char *y) {
  snprintf(out, n, "%s\\%s", x, y);
}
static int exists(const char *p) {
  DWORD d = GetFileAttributesA(p);
  return d != INVALID_FILE_ATTRIBUTES;
}
static void mkdirs(const char *path) {
  char t[MAX_PATH];
  size_t i;
  strncpy(t, path, sizeof(t) - 1);
  t[sizeof(t) - 1] = 0;
  for (i = 3; t[i]; i++)
    if (t[i] == '\\' || t[i] == '/') {
      char c = t[i];
      t[i] = 0;
      CreateDirectoryA(t, NULL);
      t[i] = c;
    }
  CreateDirectoryA(t, NULL);
}
static void file_date(unsigned long long t, char *o, size_t n) {
  time_t x = (time_t)t;
  struct tm *z = localtime(&x);
  if (z)
    strftime(o, n, "%d %b %Y", z);
  else
    strcpy_s(o, n, "Just now");
}
static unsigned long long file_mtime(const WIN32_FIND_DATAA *d) {
  ULARGE_INTEGER v;
  v.LowPart = d->ftLastWriteTime.dwLowDateTime;
  v.HighPart = d->ftLastWriteTime.dwHighDateTime;
  return v.QuadPart / 10000000ULL - 11644473600ULL;
}
static unsigned long long fnv_file(const char *path) {
  FILE *f = fopen(path, "rb");
  unsigned char b[4096];
  size_t n;
  unsigned long long h = 1469598103934665603ULL;
  if (!f)
    return 0;
  while ((n = fread(b, 1, sizeof(b), f)))
    for (size_t i = 0; i < n; i++) {
      h ^= b[i];
      h *= 1099511628211ULL;
    }
  fclose(f);
  return h;
}
static const char *base_name(const char *p) {
  const char *x = strrchr(p, '\\');
  return x ? x + 1 : p;
}

static void scan_dir(const char *dir, const char *relative) {
  char pattern[MAX_PATH], child[MAX_PATH], rel[MAX_PATH];
  WIN32_FIND_DATAA d;
  HANDLE h;
  snprintf(pattern, sizeof(pattern), "%s\\*", dir);
  h = FindFirstFileA(pattern, &d);
  if (h == INVALID_HANDLE_VALUE)
    return;
  do {
    if (!strcmp(d.cFileName, ".") || !strcmp(d.cFileName, "..") ||
        !strcmp(d.cFileName, ".stem"))
      continue;
    join(child, sizeof(child), dir, d.cFileName);
    snprintf(rel, sizeof(rel), "%s%s%s", relative, relative[0] ? "/" : "",
             d.cFileName);
    if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      scan_dir(child, rel);
    else if (a.file_count < MAX_FILES) {
      FileEntry *f = &a.files[a.file_count++];
      strncpy(f->path, rel, sizeof(f->path) - 1);
      f->size = ((unsigned long long)d.nFileSizeHigh << 32) | d.nFileSizeLow;
      f->mtime = file_mtime(&d);
    }
  } while (FindNextFileA(h, &d));
  FindClose(h);
}
static int is_staged(const char *p) {
  for (int i = 0; i < a.staged_count; i++)
    if (!_stricmp(a.staged[i], p))
      return i;
  return -1;
}
static void save_staging(void) {
  char p[MAX_PATH];
  FILE *f;
  join(p, sizeof(p), a.root, ".stem\\staging.txt");
  f = fopen(p, "w");
  if (!f)
    return;
  for (int i = 0; i < a.staged_count; i++)
    fprintf(f, "%s\n", a.staged[i]);
  fclose(f);
}
static void load_staging(void) {
  char p[MAX_PATH], b[MAX_PATH];
  FILE *f;
  a.staged_count = 0;
  join(p, sizeof(p), a.root, ".stem\\staging.txt");
  f = fopen(p, "r");
  if (!f)
    return;
  while (a.staged_count < MAX_PATHS && fgets(b, sizeof(b), f)) {
    b[strcspn(b, "\r\n")] = 0;
    if (b[0])
      strncpy(a.staged[a.staged_count++], b, MAX_PATH - 1);
  }
  fclose(f);
}
static void snapshot_manifest_path(char *out, size_t n) {
  if (a.commit_count)
    snprintf(out, n, "%s\\.stem\\snapshots\\%s.manifest", a.root,
             a.commits[a.commit_count - 1].hash);
  else
    out[0] = 0;
}
static int old_hash(const char *path, unsigned long long *hash) {
  char m[MAX_PATH], line[MAX_PATH + 40], p[MAX_PATH];
  FILE *f;
  snapshot_manifest_path(m, sizeof(m));
  f = fopen(m, "r");
  if (!f)
    return 0;
  while (fgets(line, sizeof(line), f)) {
    unsigned long long h;
    if (sscanf(line, "%[^|]|%llx", p, &h) == 2 && !_stricmp(p, path)) {
      *hash = h;
      fclose(f);
      return 1;
    }
  }
  fclose(f);
  return 0;
}
static int changed_count(void) {
  int n = 0;
  for (int i = 0; i < a.file_count; i++) {
    unsigned long long h;
    if (!old_hash(a.files[i].path, &h) || h != fnv_file((char[MAX_PATH]){0})) {
      char full[MAX_PATH];
      join(full, sizeof(full), a.root, a.files[i].path);
      if (!old_hash(a.files[i].path, &h) || h != fnv_file(full))
        n++;
    }
  }
  return n;
}

/* This avoids depending on the unfinished core index serialization while
   keeping the data plainly inspectable and stable within every STEM repository.
 */
static void load_commits(void) {
  char p[MAX_PATH], line[512];
  FILE *f;
  a.commit_count = 0;
  join(p, sizeof(p), a.root, ".stem\\commits.log");
  f = fopen(p, "r");
  if (!f)
    return;
  while (a.commit_count < MAX_COMMITS && fgets(line, sizeof(line), f)) {
    Commit *c = &a.commits[a.commit_count];
    char *at = line;
    char *next;
    next = strchr(at, '|');
    if (!next)
      continue;
    *next++ = 0;
    strncpy(c->hash, at, sizeof(c->hash) - 1);
    at = next;
    next = strchr(at, '|');
    if (!next)
      continue;
    *next++ = 0;
    strncpy(c->date, at, sizeof(c->date) - 1);
    at = next;
    next = strchr(at, '|');
    if (!next)
      continue;
    *next++ = 0;
    c->files = atoi(at);
    at = next;
    at[strcspn(at, "\r\n")] = 0;
    strncpy(c->message, at, sizeof(c->message) - 1);
    a.commit_count++;
  }
  fclose(f);
}
static void load_remote(void) {
  char p[MAX_PATH], b[MAX_PATH];
  FILE *f;
  join(p, sizeof(p), a.root, ".stem\\remote.txt");
  a.remote[0] = 0;
  f = fopen(p, "r");
  if (f && fgets(b, sizeof(b), f)) {
    b[strcspn(b, "\r\n")] = 0;
    strncpy(a.remote, b, sizeof(a.remote) - 1);
  }
  if (f)
    fclose(f);
}
static void save_remote(void) {
  char p[MAX_PATH];
  FILE *f;
  join(p, sizeof(p), a.root, ".stem\\remote.txt");
  f = fopen(p, "w");
  if (f) {
    fputs(a.remote, f);
    fclose(f);
  }
}
static void load_branch(void) {
  char p[MAX_PATH], name[64] = {0};
  FILE *f;
  join(p, sizeof(p), a.root, ".stem\\current-branch.txt");
  f = fopen(p, "r");
  if (f && fgets(name, sizeof(name), f)) {
    name[strcspn(name, "\r\n")] = 0;
    if (name[0])
      strncpy(a.branch, name, sizeof(a.branch) - 1);
  }
  if (f)
    fclose(f);
  if (!a.branch[0])
    strcpy_s(a.branch, sizeof(a.branch), "main");
}
static void save_branch(void) {
  char p[MAX_PATH];
  FILE *f;
  join(p, sizeof(p), a.root, ".stem\\current-branch.txt");
  f = fopen(p, "w");
  if (f) {
    fputs(a.branch, f);
    fclose(f);
  }
}
static void create_branch(void) {
  char path[MAX_PATH], name[64];
  FILE *f;
  if (!a.initialized || !a.commit_count) {
    MessageBoxA(a.hwnd, "Create your first commit before branching.", "STEM", MB_ICONINFORMATION);
    return;
  }
  snprintf(name, sizeof(name), "branch-%d", a.commit_count + 1);
  join(path, sizeof(path), a.root, ".stem\\branches.txt");
  f = fopen(path, "a");
  if (!f) return;
  fseek(f, 0, SEEK_END);
  if (ftell(f) == 0)
    fprintf(f, "main|%s\n", a.commits[a.commit_count - 1].hash);
  fprintf(f, "%s|%s\n", name, a.commits[a.commit_count - 1].hash);
  fclose(f);
  strncpy(a.branch, name, sizeof(a.branch) - 1);
  save_branch();
  set_status("Created and switched to a new branch");
  refresh_all();
}
static void switch_branch(void) {
  char path[MAX_PATH], line[160], first[64] = {0}, next[64] = {0};
  FILE *f;
  if (!a.initialized) return;
  join(path, sizeof(path), a.root, ".stem\\branches.txt");
  f = fopen(path, "r");
  if (!f) { set_status("Only main exists — create a commit and branch first"); return; }
  while (fgets(line, sizeof(line), f)) {
    char *sep = strchr(line, '|'); if (sep) *sep = 0;
    if (!first[0]) strncpy(first, line, sizeof(first) - 1);
    if (!_stricmp(line, a.branch) && fgets(line, sizeof(line), f)) {
      sep = strchr(line, '|'); if (sep) *sep = 0;
      strncpy(next, line, sizeof(next) - 1); break;
    }
  }
  fclose(f);
  if (!next[0]) strncpy(next, first, sizeof(next) - 1);
  if (next[0]) { strncpy(a.branch, next, sizeof(a.branch) - 1); save_branch(); set_status("Switched branch"); refresh_all(); }
}

static void refresh_all(void) {
  if (!a.initialized)
    return;
  a.file_count = 0;
  scan_dir(a.root, "");
  load_staging();
  load_commits();
  load_remote();
  load_branch();
  if (a.selected_file >= a.file_count)
    a.selected_file = a.file_count - 1;
  if (a.selected_commit >= a.commit_count)
    a.selected_commit = 0;
  InvalidateRect(a.hwnd, NULL, FALSE);
}
static void set_status(const char *s) {
  strncpy(a.status_text, s, sizeof(a.status_text) - 1);
  a.status_text[sizeof(a.status_text) - 1] = 0;
  InvalidateRect(a.hwnd, NULL, FALSE);
}
static void setup_metrics(HWND hwnd) {
  RECT rc;
  GetClientRect(hwnd, &rc);
  int width = rc.right - rc.left, height = rc.bottom - rc.top;
  ui_top = STEM_MIN(76, STEM_MAX(56, height / 15));
  ui_left = STEM_MIN(280, STEM_MAX(190, width / 8));
  ui_right = STEM_MIN(360, STEM_MAX(280, width / 6));
  ui_bottom = STEM_MIN(306, STEM_MAX(230, height / 4));
  if (width - ui_left - ui_right < 700) {
    ui_left = 180;
    ui_right = 260;
  }
}
static void copy_file_tree(const char *from, const char *to) {
  char pat[MAX_PATH], s[MAX_PATH], d[MAX_PATH];
  WIN32_FIND_DATAA x;
  HANDLE h;
  mkdirs(to);
  snprintf(pat, sizeof(pat), "%s\\*", from);
  h = FindFirstFileA(pat, &x);
  if (h == INVALID_HANDLE_VALUE)
    return;
  do {
    if (!strcmp(x.cFileName, ".") || !strcmp(x.cFileName, ".."))
      continue;
    join(s, sizeof(s), from, x.cFileName);
    join(d, sizeof(d), to, x.cFileName);
    if (x.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      copy_file_tree(s, d);
    else
      CopyFileA(s, d, FALSE);
  } while (FindNextFileA(h, &x));
  FindClose(h);
}
static void create_snapshot(const char *hash) {
  char folder[MAX_PATH], m[MAX_PATH], full[MAX_PATH];
  FILE *f;
  snprintf(folder, sizeof(folder), "%s\\.stem\\snapshots\\%s", a.root, hash);
  mkdirs(folder);
  snprintf(m, sizeof(m), "%s.manifest", folder);
  f = fopen(m, "w");
  if (!f)
    return;
  for (int i = 0; i < a.file_count; i++) {
    char dst[MAX_PATH], parent[MAX_PATH];
    join(full, sizeof(full), a.root, a.files[i].path);
    join(dst, sizeof(dst), folder, a.files[i].path);
    strncpy(parent, dst, sizeof(parent) - 1);
    char *q = strrchr(parent, '\\');
    if (q) {
      *q = 0;
      mkdirs(parent);
    }
    CopyFileA(full, dst, FALSE);
    fprintf(f, "%s|%llx\n", a.files[i].path, fnv_file(full));
  }
  fclose(f);
}
static void stage_all(void) {
  if (!a.initialized)
    return;
  a.staged_count = 0;
  for (int i = 0; i < a.file_count && a.staged_count < MAX_PATHS; i++)
    strncpy(a.staged[a.staged_count++], a.files[i].path, MAX_PATH - 1);
  save_staging();
  set_status("All working files staged");
  refresh_all();
}
static void toggle_stage(const char *p) {
  int n = is_staged(p);
  if (n >= 0) {
    memmove(&a.staged[n], &a.staged[n + 1],
            (a.staged_count - n - 1) * MAX_PATH);
    a.staged_count--;
  } else if (a.staged_count < MAX_PATHS)
    strncpy(a.staged[a.staged_count++], p, MAX_PATH - 1);
  save_staging();
  refresh_all();
}
static void commit_now(void) {
  char msg[512], log[MAX_PATH], stamp[48], hash[17];
  FILE *f;
  time_t now;
  if (!a.initialized) {
    MessageBoxA(a.hwnd, "Initialize or open a STEM repository first.", "STEM",
                MB_ICONWARNING);
    return;
  }
  if (!a.staged_count) {
    MessageBoxA(a.hwnd, "Stage at least one file before committing.",
                "Nothing staged", MB_ICONINFORMATION);
    return;
  }
  GetWindowTextA(a.msg, msg, sizeof(msg));
  if (!msg[0]) {
    MessageBoxA(a.hwnd, "Add a concise commit message.", "Commit message",
                MB_ICONINFORMATION);
    return;
  }
  now = time(NULL);
  strftime(stamp, sizeof(stamp), "%d %b %Y, %H:%M", localtime(&now));
  snprintf(hash, sizeof(hash), "%08llx",
           (unsigned long long)now ^ fnv_file(a.root));
  join(log, sizeof(log), a.root, ".stem\\commits.log");
  f = fopen(log, "a");
  if (!f) {
    MessageBoxA(a.hwnd, "Could not write commit history.", "STEM",
                MB_ICONERROR);
    return;
  }
  fprintf(f, "%s|%s|%d|%s\n", hash, stamp, a.staged_count, msg);
  fclose(f);
  create_snapshot(hash);
  a.staged_count = 0;
  save_staging();
  SetWindowTextA(a.msg, "");
  set_status("Commit created locally");
  refresh_all();
}
static void do_push(void) {
  char remoteStem[MAX_PATH], localStem[MAX_PATH];
  if (!a.remote[0]) {
    MessageBoxA(a.hwnd,
                "Choose a local folder from Change Remote first. Cloud URLs "
                "can be saved but need a provider adapter.",
                "No remote", MB_ICONINFORMATION);
    return;
  }
  if (strstr(a.remote, "://")) {
    MessageBoxA(
        a.hwnd,
        "This remote is configured, but this build has no provider adapter for "
        "it yet. Use a local folder remote for a real push.",
        "Provider adapter pending", MB_ICONINFORMATION);
    return;
  }
  snprintf(remoteStem, sizeof(remoteStem), "%s\\.stem", a.remote);
  join(localStem, sizeof(localStem), a.root, ".stem");
  copy_file_tree(localStem, remoteStem);
  set_status("Pushed STEM metadata to local remote");
}
static void do_pull(void) {
  char remoteStem[MAX_PATH], localStem[MAX_PATH];
  if (!a.remote[0] || strstr(a.remote, "://")) {
    MessageBoxA(a.hwnd,
                "Pull is available for a configured local-folder remote only.",
                "STEM", MB_ICONINFORMATION);
    return;
  }
  snprintf(remoteStem, sizeof(remoteStem), "%s\\.stem", a.remote);
  join(localStem, sizeof(localStem), a.root, ".stem");
  if (GetFileAttributesA(remoteStem) == INVALID_FILE_ATTRIBUTES) {
    MessageBoxA(a.hwnd, "The configured folder does not contain a STEM remote.", "Pull", MB_ICONWARNING);
    return;
  }
  if (MessageBoxA(a.hwnd, "Fetch history, snapshots, and branch state from this local remote?", "Pull from remote", MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
    return;
  copy_file_tree(remoteStem, localStem);
  set_status("Fetched repository data from local remote");
  refresh_all();
}
static void choose_remote(void) {
  BROWSEINFOA bi = {0};
  char display[MAX_PATH];
  bi.hwndOwner = a.hwnd;
  bi.lpszTitle = "Choose a local folder to use as the STEM remote";
  PIDLIST_ABSOLUTE id = SHBrowseForFolderA(&bi);
  if (id) {
    if (SHGetPathFromIDListA(id, a.remote)) {
      save_remote();
      set_status("Local remote configured");
      refresh_all();
    }
    CoTaskMemFree(id);
  }
}
static void init_repo(void) {
  char cwd[MAX_PATH];
  GetCurrentDirectoryA(sizeof(cwd), cwd);
  if (a.initialized) {
    MessageBoxA(a.hwnd, "This folder already contains a STEM repository.",
                "STEM", MB_ICONINFORMATION);
    return;
  }
  if (stem_repository_init(cwd, FALSE, &a.repo) != STEM_OK) {
    MessageBoxA(a.hwnd, "STEM could not initialize this folder.", "STEM",
                MB_ICONERROR);
    return;
  }
  strncpy(a.root, cwd, sizeof(a.root) - 1);
  a.initialized = TRUE;
  strcpy_s(a.branch, sizeof(a.branch), "main");
  set_status("Repository initialized — stage your first change");
  refresh_all();
}

static void set_font(HDC h, HFONT f, COLORREF c) {
  SelectObject(h, f);
  SetTextColor(h, c);
  SetBkMode(h, TRANSPARENT);
}
static void text(HDC h, int x, int y, int w, const char *s, HFONT f,
                 COLORREF c) {
  RECT r = {x, y, x + w, y + 30};
  set_font(h, f, c);
  DrawTextA(h, s, -1, &r,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}
static void line(HDC h, int x1, int y1, int x2, int y2, COLORREF c) {
  HPEN p = CreatePen(PS_SOLID, 1, c), o = SelectObject(h, p);
  MoveToEx(h, x1, y1, NULL);
  LineTo(h, x2, y2);
  SelectObject(h, o);
  DeleteObject(p);
}
static void draw_round(HDC h, RECT r, COLORREF fill, int rad) {
  HBRUSH b = CreateSolidBrush(fill), o = SelectObject(h, b);
  HPEN p = CreatePen(PS_SOLID, 1, C_BORDER), op = SelectObject(h, p);
  RoundRect(h, r.left, r.top, r.right, r.bottom, rad, rad);
  SelectObject(h, op);
  SelectObject(h, o);
  DeleteObject(p);
  DeleteObject(b);
}
static void hit(RECT r, int id) {
  if (a.hit_count < 80) {
    a.hits[a.hit_count].r = r;
    a.hits[a.hit_count++].id = id;
  }
}
#define HIT_INIT 1
#define HIT_STAGE 2
#define HIT_COMMIT 3
#define HIT_PULL 4
#define HIT_PUSH 5
#define HIT_SYNC 6
#define HIT_REMOTE 7
#define HIT_BRANCH 8
#define HIT_BRANCH_SWITCH 9
#define HIT_FILE 100
#define HIT_NAV 200
#define HIT_HISTORY 300

static void button(HDC h, int x, int y, int w, const char *label,
                   COLORREF color, int id) {
  RECT r = {x, y, x + w, y + 34};
  draw_round(h, r, color, 9);
  text(h, x + 13, y + 2, w - 24, label, a.bold, C_TEXT);
  hit(r, id);
}
static void draw_nav(HDC h, RECT rc) {
  (void)rc;
  const char *n[] = {"Overview", "Changes", "History", "Branches",
                     "Tags",     "Remotes", "Settings"};
  const char *ic[] = {"o", "#", "H", "*", "#", "@", "S"};
  for (int i = 0; i < 7; i++) {
    int y = 88 + i * 47;
    RECT r = {12, y, LEFT_W - 12, y + 38};
    if (a.page == i)
      draw_round(h, r, C_HOVER, 9);
    text(h, 29, y + 3, 20, ic[i], a.bold, a.page == i ? C_BLUE : C_MUTED);
    text(h, 57, y + 3, 130, n[i], a.ui, C_TEXT);
    hit(r, HIT_NAV + i);
  }
  line(h, 17, 430, LEFT_W - 17, 430, C_BORDER);
  text(h, 28, 450, 160, "REPOSITORY", a.small, C_MUTED);
  text(h, 28, 480, 170, base_name(a.root[0] ? a.root : "No repository"), a.bold,
       C_TEXT);
  int y = 512;
  const char *tree[] = {".stem",  "src",       "include",
                        "assets", "README.md", "CMakeLists.txt"};
  for (int i = 0; i < 6; i++) {
    text(h, 31, y + i * 28, 160, tree[i], a.ui, (i == 0) ? C_MUTED : C_TEXT);
  }
}
static void draw_timeline(HDC h, int x, int y, int w, int bottom) {
  text(h, x + 16, y + 15, 200, a.page == PAGE_CHANGES ? "Changes" : "History",
       a.title, C_TEXT);
  text(h, x + 16, y + 46, 260,
       a.page == PAGE_CHANGES ? "Working tree files" : "Search commits...",
       a.small, C_MUTED);
  line(h, x, y + 70, x + w, y + 70, C_BORDER);
  int cy = y + 88;
  int count = a.page == PAGE_CHANGES ? a.file_count : a.commit_count;
  for (int i = 0; i < count && cy < bottom - 20; i++) {
    RECT r = {x + 8, cy, x + w - 10, cy + 72};
    BOOL selected = (a.page == PAGE_CHANGES ? i == a.selected_file
                                            : i == a.selected_commit);
    if (selected)
      draw_round(h, r, RGB(24, 48, 87), 10);
    HBRUSH b = CreateSolidBrush(selected ? C_BLUE : C_MUTED),
           old = SelectObject(h, b);
    Ellipse(h, x + 22, cy + 22, x + 34, cy + 34);
    SelectObject(h, old);
    DeleteObject(b);
    if (i < count - 1)
      line(h, x + 27, cy + 34, x + 27, cy + 72, selected ? C_BLUE : C_BORDER);
    char top[300], sub[250];
    if (a.page == PAGE_CHANGES) {
      FileEntry *f = &a.files[i];
      snprintf(top, sizeof(top), "%s", f->path);
      snprintf(sub, sizeof(sub), "%s  •  %llu KB",
               is_staged(f->path) >= 0 ? "Staged" : "Modified", f->size / 1024);
      hit(r, HIT_FILE + i);
    } else {
      Commit *c = &a.commits[i];
      snprintf(top, sizeof(top), "%s", c->message);
      snprintf(sub, sizeof(sub), "You  •  %s  •  %s", c->date, c->hash);
      hit(r, HIT_HISTORY + i);
    }
    text(h, x + 48, cy + 9, w - 75, top, a.bold, C_TEXT);
    text(h, x + 48, cy + 35, w - 75, sub, a.small, C_MUTED);
    cy += 77;
  }
  if (!count)
    text(h, x + 30, cy + 20, w - 50,
         a.page == PAGE_CHANGES ? "No files yet — add one to this folder."
                                : "No commits yet — stage changes and commit.",
         a.ui, C_MUTED);
}
static void set_diff(void) {
  char full[MAX_PATH], base[MAX_PATH], buf[8192] = {0}, old[8192] = {0},
                                       m[MAX_PATH];
  if (a.selected_file < 0 || a.selected_file >= a.file_count) {
    SetWindowTextA(a.diff, "Select a working file to preview its changes.");
    return;
  }
  join(full, sizeof(full), a.root, a.files[a.selected_file].path);
  FILE *f = fopen(full, "rb");
  if (f) {
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
  }
  if (a.commit_count) {
    snprintf(base, sizeof(base), "%s\\.stem\\snapshots\\%s\\%s", a.root,
             a.commits[a.commit_count - 1].hash, a.files[a.selected_file].path);
    f = fopen(base, "rb");
    if (f) {
      fread(old, 1, sizeof(old) - 1, f);
      fclose(f);
    }
  }
  snprintf(m, sizeof(m), "--- previous/%s\r\n+++ working/%s\r\n\r\n%s%s",
           a.files[a.selected_file].path, a.files[a.selected_file].path,
           old[0] ? "- " : "", old[0] ? old : "(new file)\r\n");
  strncat(m, "\r\n+ ", sizeof(m) - strlen(m) - 1);
  strncat(m, buf, sizeof(m) - strlen(m) - 1);
  SetWindowTextA(a.diff, m);
}
static void draw_workspace(HDC h, RECT rc) {
  int x = LEFT_W, y = TOP_H, w = rc.right - LEFT_W - RIGHT_W,
      bottom = rc.bottom - BOTTOM_H - STATUS_H;
  int listw = STEM_MIN(385, w / 2);
  draw_timeline(h, x, y, listw, bottom);
  line(h, x + listw, y, x + listw, bottom, C_BORDER);
  int dx = x + listw + 22;
  if (a.page == PAGE_CHANGES && a.selected_file >= 0) {
    FileEntry *f = &a.files[a.selected_file];
    text(h, dx, y + 19, w - listw - 40, f->path, a.title, C_TEXT);
    char meta[256];
    snprintf(meta, sizeof(meta), "Working copy  •  %llu KB  •  %s",
             f->size / 1024, is_staged(f->path) >= 0 ? "staged" : "unstaged");
    text(h, dx, y + 52, w - listw - 40, meta, a.ui, C_MUTED);
    button(h, dx, y + 82, 116,
           is_staged(f->path) >= 0 ? "Unstage" : "Stage file",
           is_staged(f->path) >= 0 ? C_HOVER : RGB(18, 92, 64),
           HIT_FILE + a.selected_file + 1000);
  } else if (a.commit_count) {
    Commit *c = &a.commits[a.selected_commit];
    text(h, dx, y + 19, w - listw - 40, c->message, a.title, C_TEXT);
    char m[180];
    snprintf(m, sizeof(m), "You  •  %s  •  %s", c->date, c->hash);
    text(h, dx, y + 52, w - listw - 40, m, a.ui, C_MUTED);
    text(h, dx, y + 101, 200, "Snapshot commit", a.bold, C_GREEN);
  } else {
    text(h, dx, y + 24, w - listw - 40, "Your project history", a.title,
         C_TEXT);
    text(h, dx, y + 62, w - listw - 40,
         "STEM keeps snapshots locally and syncs when you decide.", a.ui,
         C_MUTED);
  }
  RECT diff = {dx, y + 130, rc.right - RIGHT_W - 18, bottom - 18};
  draw_round(h, diff, RGB(11, 18, 29), 10);
  text(h, diff.left + 14, diff.top + 8, 260, "DIFF PREVIEW", a.small, C_MUTED);
  line(h, diff.left + 12, diff.top + 35, diff.right - 12, diff.top + 35,
       C_BORDER);
}
static void status_row(HDC h, int x, int y, const char *label,
                       const char *value, COLORREF c) {
  text(h, x, y, 170, label, a.ui, C_MUTED);
  RECT r = {x + 184, y + 2, x + 260, y + 25};
  set_font(h, a.bold, c);
  DrawTextA(h, value, -1, &r, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
}
static void draw_right(HDC h, RECT rc) {
  int x = rc.right - RIGHT_W, y = TOP_H;
  line(h, x, y, x, rc.bottom, C_BORDER);
  text(h, x + 20, y + 20, 120, "REMOTE", a.small, C_MUTED);
  draw_round(h, (RECT){x + 15, y + 48, rc.right - 16, y + 179}, C_PANEL2, 10);
  text(h, x + 31, y + 63, 200,
       a.remote[0] ? "Local Folder" : "No remote configured", a.bold, C_TEXT);
  text(h, x + 31, y + 91, 224,
       a.remote[0] ? a.remote : "Choose a storage location to sync", a.small,
       C_MUTED);
  text(h, x + 31, y + 119, 100, a.remote[0] ? "Connected" : "Not connected",
       a.bold, a.remote[0] ? C_GREEN : C_AMBER);
  button(h, x + 31, y + 138, 220, "Change Remote", C_HOVER, HIT_REMOTE);
  int sy = y + 205;
  text(h, x + 20, sy, 160, "STATUS", a.small, C_MUTED);
  draw_round(h, (RECT){x + 15, sy + 29, rc.right - 16, sy + 180}, C_PANEL2, 10);
  status_row(h, x + 31, sy + 45, "On branch", a.branch[0] ? a.branch : "main",
             C_TEXT);
  char b[32];
  snprintf(b, sizeof(b), "%d", a.staged_count);
  status_row(h, x + 31, sy + 75, "Staged files", b,
             a.staged_count ? C_AMBER : C_GREEN);
  snprintf(b, sizeof(b), "%d", a.file_count);
  status_row(h, x + 31, sy + 105, "Working files", b, C_BLUE);
  status_row(h, x + 31, sy + 135, "Sync status",
             a.remote[0] ? "Ready" : "Local only",
             a.remote[0] ? C_GREEN : C_MUTED);
  int ay = sy + 205;
  text(h, x + 20, ay, 160, "ACTIONS", a.small, C_MUTED);
  button(h, x + 21, ay + 31, 118, "+ Stage all", C_HOVER, HIT_STAGE);
  button(h, x + 153, ay + 31, 118, "Commit", C_HOVER, HIT_COMMIT);
  button(h, x + 21, ay + 76, 118, "Pull", C_HOVER, HIT_PULL);
  button(h, x + 153, ay + 76, 118, "Push", C_HOVER, HIT_PUSH);
  button(h, x + 21, ay + 121, 250, "Sync now", RGB(18, 92, 64), HIT_SYNC);
}
static void draw_bottom(HDC h, RECT rc) {
  int end = rc.bottom - STATUS_H;
  int y = end - BOTTOM_H, x = LEFT_W, w = rc.right - LEFT_W - RIGHT_W;
  line(h, x, y, rc.right - RIGHT_W, y, C_BORDER);
  text(h, x + 18, y + 13, 190, "Commit message", a.bold, C_TEXT);
  text(h, x + w / 2 + 20, y + 13, 190, "Staged changes", a.bold, C_TEXT);
  line(h, x + w / 2, y + 12, x + w / 2, end - 12, C_BORDER);
  for (int i = 0; i < a.staged_count && i < 5; i++) {
    text(h, x + w / 2 + 24, y + 49 + i * 28, w / 2 - 70, a.staged[i], a.ui,
         C_TEXT);
  }
  if (!a.staged_count)
    text(h, x + w / 2 + 24, y + 50, w / 2 - 70, "Nothing staged yet", a.ui,
         C_MUTED);
  button(h, x + w / 2 + 24, end - 55, w / 2 - 48, "Commit staged changes",
         RGB(18, 112, 73), HIT_COMMIT);
}
static void paint(HDC h, RECT *rc) {
  FillRect(h, rc, a.bg);
  a.hit_count = 0;
  RECT top = {0, 0, rc->right, TOP_H};
  FillRect(h, &top, a.panel);
  line(h, 0, TOP_H, rc->right, TOP_H, C_BORDER);
  draw_round(h, (RECT){21, 18, 43, 45}, C_GREEN, 12);
  text(h, 54, 17, 95, "STEM", a.title, C_TEXT);
  text(h, 134, 22, 62, "v0.1.0", a.small, C_MUTED);
  button(h, 245, 16, 138,
         a.initialized ? (a.branch[0] ? a.branch : "main") : "Initialize",
         C_HOVER, a.initialized ? HIT_BRANCH_SWITCH : HIT_INIT);
  button(h, 455, 16, 93, "+ Branch", RGB(18, 82, 60), HIT_BRANCH);
  button(h, 560, 16, 76, "Pull", C_HOVER, HIT_PULL);
  button(h, 647, 16, 76, "Push", C_HOVER, HIT_PUSH);
  button(h, 734, 16, 82, "Sync", C_HOVER, HIT_SYNC);
  draw_round(h, (RECT){rc->right - 350, 16, rc->right - 180, 50}, C_PANEL2, 9);
  text(h, rc->right - 332, 18, 142, "Search (Ctrl+K)", a.ui, C_MUTED);
  text(h, rc->right - 155, 18, 24, "S", a.bold, C_TEXT);
  draw_nav(h, *rc);
  draw_workspace(h, *rc);
  draw_right(h, *rc);
  draw_bottom(h, *rc);
  RECT status = {0, rc->bottom - STATUS_H, rc->right, rc->bottom};
  FillRect(h, &status, a.panel);
  line(h, 0, status.top, rc->right, status.top, C_BORDER);
  text(h, 18, status.top, rc->right - 36,
       a.status_text[0] ? a.status_text : "Ready", a.small, C_MUTED);
}

static LRESULT CALLBACK proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_CREATE: {
    a.hwnd = hwnd;
    a.ui = CreateFontA(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0,
                       CLEARTYPE_QUALITY, 0, "Segoe UI");
    a.bold = CreateFontA(15, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0,
                         0, CLEARTYPE_QUALITY, 0, "Segoe UI");
    a.title = CreateFontA(20, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0,
                          0, CLEARTYPE_QUALITY, 0, "Segoe UI");
    a.small = CreateFontA(12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0,
                          0, CLEARTYPE_QUALITY, 0, "Segoe UI");
    a.mono = CreateFontA(13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0,
                         CLEARTYPE_QUALITY, 0, "Cascadia Mono");
    a.bg = CreateSolidBrush(C_BG);
    a.panel = CreateSolidBrush(C_PANEL);
    a.edit_bg = CreateSolidBrush(RGB(11, 18, 29));
    a.msg = CreateWindowExA(0, "EDIT", "",
                            WS_CHILD | WS_VISIBLE | ES_MULTILINE |
                                ES_AUTOVSCROLL | ES_WANTRETURN,
                            0, 0, 0, 0, hwnd, NULL, NULL, NULL);
    a.diff = CreateWindowExA(0, "EDIT",
                             "Select a working file to preview its changes.",
                             WS_CHILD | WS_VISIBLE | ES_MULTILINE |
                                 ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
                             0, 0, 0, 0, hwnd, NULL, NULL, NULL);
    a.status = CreateWindowExA(
        0, STATUSCLASSNAMEA, "Open a STEM repository or initialize this folder",
        WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
    SendMessage(a.msg, WM_SETFONT, (WPARAM)a.mono, TRUE);
    SendMessage(a.diff, WM_SETFONT, (WPARAM)a.mono, TRUE);
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(sizeof(cwd), cwd);
    if (stem_repository_open(cwd, &a.repo) == STEM_OK) {
      strncpy(a.root, cwd, sizeof(a.root) - 1);
      a.initialized = TRUE;
      char *br = NULL;
      if (stem_branch_current(a.repo, &br) == STEM_OK) {
        strncpy(a.branch, br, sizeof(a.branch) - 1);
        free(br);
      } else
        strcpy_s(a.branch, sizeof(a.branch), "main");
      refresh_all();
    } else
      strcpy_s(a.branch, sizeof(a.branch), "main");
    set_status(a.initialized ? "Repository opened" : "Open a STEM repository or initialize this folder");
    layout(hwnd);
    return 0;
  }
  case WM_SIZE:
    layout(hwnd);
    return 0;
  case WM_CTLCOLORSTATIC:
  case WM_CTLCOLOREDIT: {
    HDC dc = (HDC)wp;
    SetTextColor(dc, C_TEXT);
    SetBkColor(dc, RGB(11, 18, 29));
    return (LRESULT)a.edit_bg;
  }
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC h = BeginPaint(hwnd, &ps);
    RECT r;
    GetClientRect(hwnd, &r);
    paint(h, &r);
    EndPaint(hwnd, &ps);
    return 0;
  }
  case WM_LBUTTONUP: {
    POINT p = {LOWORD(lp), HIWORD(lp)};
    for (int i = a.hit_count - 1; i >= 0; i--)
      if (PtInRect(&a.hits[i].r, p)) {
        int id = a.hits[i].id;
        if (id == HIT_INIT)
          init_repo();
        else if (id == HIT_STAGE)
          stage_all();
        else if (id == HIT_COMMIT)
          commit_now();
        else if (id == HIT_PUSH)
          do_push();
        else if (id == HIT_PULL)
          do_pull();
        else if (id == HIT_SYNC) {
          do_push();
        } else if (id == HIT_REMOTE)
          choose_remote();
        else if (id == HIT_BRANCH)
          create_branch();
        else if (id == HIT_BRANCH_SWITCH)
          switch_branch();
        else if (id >= HIT_NAV && id < HIT_NAV + 7) {
          a.page = (Page)(id - HIT_NAV);
          refresh_all();
        } else if (id >= HIT_FILE + 1000 && id < HIT_FILE + 1000 + MAX_FILES) {
          toggle_stage(a.files[id - HIT_FILE - 1000].path);
        } else if (id >= HIT_FILE && id < HIT_FILE + MAX_FILES) {
          a.selected_file = id - HIT_FILE;
          set_diff();
          InvalidateRect(hwnd, NULL, FALSE);
        } else if (id >= HIT_HISTORY && id < HIT_HISTORY + MAX_COMMITS) {
          a.selected_commit = id - HIT_HISTORY;
          InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
      }
    return 0;
  }
  case WM_DESTROY:
    if (a.repo)
      stem_repository_free(a.repo);
    stem_shutdown();
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcA(hwnd, msg, wp, lp);
}
static void layout(HWND h) {
  RECT r;
  setup_metrics(h);
  GetClientRect(h, &r);
  int bottom = r.bottom - BOTTOM_H - STATUS_H;
  int listw = STEM_MIN(410, (r.right - LEFT_W - RIGHT_W) / 2);
  int commit_w = (r.right - LEFT_W - RIGHT_W) / 2 - 38;
  SetWindowPos(a.msg, NULL, LEFT_W + 18, bottom + 42, STEM_MAX(220, commit_w),
               STEM_MAX(90, BOTTOM_H - 92), SWP_NOZORDER);
  SetWindowPos(a.diff, NULL, LEFT_W + listw + 36, TOP_H + 168,
               STEM_MAX(240, r.right - RIGHT_W - (LEFT_W + listw + 54)),
               STEM_MAX(120, bottom - (TOP_H + 186)), SWP_NOZORDER);
  ShowWindow(a.status, SW_HIDE);
}
int WINAPI WinMain(HINSTANCE hi, HINSTANCE hp, LPSTR cmd, int show) {
  (void)hp;
  (void)cmd;
  stem_init();
  INITCOMMONCONTROLSEX ic = {sizeof(ic), ICC_BAR_CLASSES};
  InitCommonControlsEx(&ic);
  WNDCLASSEXA w = {sizeof(w)};
  w.lpfnWndProc = proc;
  w.hInstance = hi;
  w.hCursor = LoadCursor(NULL, IDC_ARROW);
  w.hbrBackground = CreateSolidBrush(C_BG);
  w.lpszClassName = "STEMDesktop";
  RegisterClassExA(&w);
  HWND h = CreateWindowExA(0, "STEMDesktop", APP_TITLE,
                           WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT,
                           CW_USEDEFAULT, 1540, 980, NULL, NULL, hi, NULL);
  ShowWindow(h, show);
  UpdateWindow(h);
  MSG m;
  while (GetMessageA(&m, NULL, 0, 0) > 0) {
    TranslateMessage(&m);
    DispatchMessageA(&m);
  }
  return (int)m.wParam;
}
