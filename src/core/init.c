#include "stem/stem.h"
#include <string.h>
#include <stdio.h>

#define STEM_VERSION_MAJOR 1
#define STEM_VERSION_MINOR 0
#define STEM_VERSION_PATCH 0

static bool g_initialized = false;
static char g_last_error[512] = {0};

stem_error_t stem_init(void) {
    if (g_initialized) {
        return STEM_OK;
    }
    
    /* Initialize OpenSSL if needed */
    /* Initialize other subsystems */
    
    g_initialized = true;
    return STEM_OK;
}

void stem_shutdown(void) {
    if (!g_initialized) {
        return;
    }
    
    /* Cleanup subsystems */
    g_initialized = false;
}

const char* stem_version(void) {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d",
             STEM_VERSION_MAJOR, STEM_VERSION_MINOR, STEM_VERSION_PATCH);
    return version;
}

int stem_version_major(void) {
    return STEM_VERSION_MAJOR;
}

int stem_version_minor(void) {
    return STEM_VERSION_MINOR;
}

int stem_version_patch(void) {
    return STEM_VERSION_PATCH;
}

const char* stem_last_error(void) {
    return g_last_error[0] ? g_last_error : NULL;
}

void stem_set_last_error(const char *msg) {
    if (msg) {
        strncpy(g_last_error, msg, sizeof(g_last_error) - 1);
        g_last_error[sizeof(g_last_error) - 1] = '\0';
    } else {
        g_last_error[0] = '\0';
    }
}
