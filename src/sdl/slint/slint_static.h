#ifndef WC2_SLINT_STATIC_H
#define WC2_SLINT_STATIC_H

#include <private/slint_config.h>

/* Slint marks its Windows item accessors dllimport even for a static build.
 * Load its configuration first, then use ordinary declarations in every
 * launcher translation unit, including the generated Slint source. */
#ifdef _WIN32
#undef SLINT_DLL_IMPORT
#define SLINT_DLL_IMPORT
#endif

#endif
