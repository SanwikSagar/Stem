#include "stem/stem.h"
#include "commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    const char *alias;
    const char *command;
} command_alias_t;

static const char *k_commands[] = {
    "init", "clone", "add", "commit", "push", "pull", "sync", "status",
    "history", "diff", "branch", "switch", "merge", "restore", "verify",
    "doctor", "remote", "config", "tag", "help", "version", "features"
};

static const command_alias_t k_aliases[] = {
    {"st", "status"},
    {"ci", "commit"},
    {"co", "switch"},
    {"br", "branch"},
    {"lg", "history"},
    {"log", "history"},
    {"df", "diff"},
    {"rs", "restore"},
    {"rb", "remote"},
    {"dr", "doctor"}
};

static size_t command_count(void) {
    return sizeof(k_commands) / sizeof(k_commands[0]);
}

static size_t alias_count(void) {
    return sizeof(k_aliases) / sizeof(k_aliases[0]);
}

static int min3(int a, int b, int c) {
    int m = (a < b) ? a : b;
    return (m < c) ? m : c;
}

static int edit_distance(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    size_t i, j;

    int *prev = (int *)malloc((lb + 1) * sizeof(int));
    int *curr = (int *)malloc((lb + 1) * sizeof(int));

    if (!prev || !curr) {
        free(prev);
        free(curr);
        return 999;
    }

    for (j = 0; j <= lb; j++) {
        prev[j] = (int)j;
    }

    for (i = 1; i <= la; i++) {
        curr[0] = (int)i;
        for (j = 1; j <= lb; j++) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            curr[j] = min3(
                prev[j] + 1,
                curr[j - 1] + 1,
                prev[j - 1] + cost
            );
        }

        for (j = 0; j <= lb; j++) {
            prev[j] = curr[j];
        }
    }

    i = (size_t)prev[lb];
    free(prev);
    free(curr);
    return (int)i;
}

static const char *resolve_alias(const char *command) {
    size_t i;
    for (i = 0; i < alias_count(); i++) {
        if (strcmp(command, k_aliases[i].alias) == 0) {
            return k_aliases[i].command;
        }
    }
    return command;
}

static const char *find_closest_command(const char *input) {
    size_t i;
    const char *best = NULL;
    int best_score = 999;

    for (i = 0; i < command_count(); i++) {
        int score = edit_distance(input, k_commands[i]);
        if (score < best_score) {
            best_score = score;
            best = k_commands[i];
        }
    }

    if (!best) {
        return NULL;
    }

    if (best_score <= 2) {
        return best;
    }

    if (best_score <= 3 && strlen(input) >= 5) {
        return best;
    }

    return NULL;
}

static void print_aliases(void) {
    size_t i;
    printf("Quick aliases:\n");
    for (i = 0; i < alias_count(); i++) {
        printf("  %-8s -> %s\n", k_aliases[i].alias, k_aliases[i].command);
    }
    printf("\n");
}

static void print_features(void) {
    printf("\n");
    printf("STEM Advanced Features\n");
    printf("======================\n\n");

    printf("Core:\n");
    printf("  - Init, add, commit, status, restore, verify\n");
    printf("  - Branching, switching, merging, tags\n");
    printf("  - History and diff inspection\n\n");

    printf("Cloud:\n");
    printf("  - Google Drive OAuth2 auth and token refresh\n");
    printf("  - Compressed archive push/pull with .stemignore support\n");
    printf("  - Upload verification and incomplete remote-file cleanup\n");
    printf("  - Live upload progress with bar, speed, and ETA\n\n");

    printf("Power-user UX:\n");
    printf("  - Command aliases (st, ci, co, br, lg, log, df, rs, rb, dr)\n");
    printf("  - Typo suggestions for unknown commands\n");
    printf("  - Doctor diagnostics and config management\n\n");

    printf("Use 'stem help' for command reference.\n\n");
}

void print_usage(void) {
    printf("STEM - The Next Generation Distributed Version Control System\n\n");
    printf("Usage: stem <command> [options]\n\n");
    printf("Core Commands:\n");
    printf("  init              Initialize a new repository\n");
    printf("  clone <url>       Clone from Google Drive\n");
    printf("  add <files>       Stage files for commit\n");
    printf("  commit            Commit staged changes\n");
    printf("  status            Show repository status\n\n");
    printf("Branching Commands:\n");
    printf("  branch [name]     List, create, or delete branches\n");
    printf("  switch <branch>   Switch to a branch\n");
    printf("  merge <branch>    Merge a branch\n\n");
    printf("Tagging Commands:\n");
    printf("  tag [name]        List, create, or delete tags\n\n");
    printf("History Commands:\n");
    printf("  history           Show commit history\n");
    printf("  diff [file]       Show differences\n");
    printf("  restore <files>   Restore files from last commit\n");
    printf("  verify            Verify repository integrity\n\n");
    printf("Remote Commands:\n");
    printf("  remote            Manage remote storage\n");
    printf("  push              Push to cloud (compressed)\n");
    printf("  pull              Pull from cloud\n");
    printf("  sync              Sync with cloud (pull + push)\n\n");
    printf("Configuration:\n");
    printf("  config            Get/set configuration values\n");
    printf("  doctor            Run repository diagnostics\n\n");
    printf("Other Commands:\n");
    printf("  help              Show this help message\n");
    printf("  features          Show advanced feature overview\n");
    printf("  version           Show version information\n\n");
    printf("Examples:\n");
    printf("  stem init\n");
    printf("  stem config user.name \"Your Name\"\n");
    printf("  stem add .\n");
    printf("  stem commit -m \"Initial commit\"\n");
    printf("  stem branch feature-x\n");
    printf("  stem branch -d old-feature\n");
    printf("  stem switch feature-x\n");
    printf("  stem merge main\n");
    printf("  stem tag v1.0.0\n");
    printf("  stem tag -d old-tag\n");
    printf("  stem push\n");
    printf("  stem clone https://drive.google.com/drive/folders/ID\n\n");
    print_aliases();
    printf("For detailed help: stem <command> --help\n");
    printf("Full manual: See MANUAL.md\n");
    printf("Website: https://stem-vcs.org\n");
}

void print_version(void) {
    printf("STEM version %s\n", stem_version());
    printf("Pure C implementation for maximum performance\n");
    printf("Copyright (c) 2026 STEM VCS Contributors\n");
}

int main(int argc, char **argv) {
    /* Initialize STEM library */
    if (stem_init() != STEM_OK) {
        fprintf(stderr, "Error: Failed to initialize STEM\n");
        return 1;
    }

    /* No command provided */
    if (argc < 2) {
        print_usage();
        stem_shutdown();
        return 0;
    }

    const char *command = resolve_alias(argv[1]);

    /* Route commands */
    int result = 0;
    
    if (strcmp(command, "init") == 0) {
        result = cmd_init(argc - 1, argv + 1);
    }
    else if (strcmp(command, "clone") == 0) {
        result = cmd_clone(argc - 1, argv + 1);
    }
    else if (strcmp(command, "add") == 0) {
        result = cmd_add(argc - 1, argv + 1);
    }
    else if (strcmp(command, "commit") == 0) {
        result = cmd_commit(argc - 1, argv + 1);
    }
    else if (strcmp(command, "push") == 0) {
        result = cmd_push(argc - 1, argv + 1);
    }
    else if (strcmp(command, "pull") == 0) {
        result = cmd_pull(argc - 1, argv + 1);
    }
    else if (strcmp(command, "sync") == 0) {
        result = cmd_sync(argc - 1, argv + 1);
    }
    else if (strcmp(command, "status") == 0) {
        result = cmd_status(argc - 1, argv + 1);
    }
    else if (strcmp(command, "history") == 0) {
        result = cmd_history(argc - 1, argv + 1);
    }
    else if (strcmp(command, "diff") == 0) {
        result = cmd_diff(argc - 1, argv + 1);
    }
    else if (strcmp(command, "branch") == 0) {
        result = cmd_branch(argc - 1, argv + 1);
    }
    else if (strcmp(command, "switch") == 0) {
        result = cmd_switch(argc - 1, argv + 1);
    }
    else if (strcmp(command, "merge") == 0) {
        result = cmd_merge(argc - 1, argv + 1);
    }
    else if (strcmp(command, "restore") == 0) {
        result = cmd_restore(argc - 1, argv + 1);
    }
    else if (strcmp(command, "verify") == 0) {
        result = cmd_verify(argc - 1, argv + 1);
    }
    else if (strcmp(command, "doctor") == 0) {
        result = cmd_doctor(argc - 1, argv + 1);
    }
    else if (strcmp(command, "remote") == 0) {
        result = cmd_remote(argc - 1, argv + 1);
    }
    else if (strcmp(command, "config") == 0) {
        result = cmd_config(argc - 1, argv + 1);
    }
    else if (strcmp(command, "tag") == 0) {
        result = cmd_tag(argc - 1, argv + 1);
    }
    else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        print_usage();
    }
    else if (strcmp(command, "version") == 0 || strcmp(command, "--version") == 0 || strcmp(command, "-v") == 0) {
        print_version();
    }
    else if (strcmp(command, "features") == 0) {
        print_features();
    }
    else {
        const char *suggestion = find_closest_command(command);
        fprintf(stderr, "Error: Unknown command '%s'\n", argv[1]);
        if (suggestion) {
            fprintf(stderr, "Did you mean '%s'?\n", suggestion);
        }
        fprintf(stderr, "Run 'stem help' for usage information\n");
        result = 1;
    }

    /* Cleanup */
    stem_shutdown();
    
    return result;
}
