#include "jconfigint.h"
#include <cstddef>
#include <cstdint>

static_assert(sizeof(size_t) == SIZEOF_SIZE_T, "The size of `size_t', as computed by sizeof.");

#if defined(HAVE_BUILTIN_CTZL)
static_assert(sizeof(unsigned long) == sizeof(size_t), "Define if your compiler has __builtin_ctzl() and sizeof(unsigned long) == sizeof(size_t).");
#endif