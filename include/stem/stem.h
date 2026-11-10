#ifndef STEM_H
#define STEM_H

/* Main STEM VCS header - includes all public APIs */

#include "stem/types.h"
#include "stem/repository.h"
#include "stem/object.h"
#include "stem/index.h"
#include "stem/reference.h"
#include "stem/config.h"
#include "stem/storage.h"
#include "stem/crypto.h"
#include "stem/merge.h"

/* Library initialization */
STEM_API stem_error_t stem_init(void);
STEM_API void stem_shutdown(void);

/* Error handling */
STEM_API const char* stem_error_message(stem_error_t error);
STEM_API const char* stem_last_error(void);

/* Version information */
STEM_API const char* stem_version(void);
STEM_API int stem_version_major(void);
STEM_API int stem_version_minor(void);
STEM_API int stem_version_patch(void);

#endif /* STEM_H */
