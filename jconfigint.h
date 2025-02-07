/* libjpeg-turbo build number */
#define BUILD  "20250207"

/* Compiler's inline keyword */
#undef inline

/* How to obtain function inlining. */
#define INLINE  __forceinline

/* How to obtain thread-local storage */
#define THREAD_LOCAL  __declspec(thread)

/* Define to the full name of this package. */
#define PACKAGE_NAME  "libjpeg-turbo"

/* Version number of package */
#define VERSION  "2.1.5.1"

/* The size of `size_t', as computed by sizeof. */
#if defined(__GNUC__)
 // GCC or CLANG
#define HAVE_BUILTIN_CTZL 
#if defined(__aarch64__) || defined(__x86_64__)
#define SIZEOF_SIZE_T 8
#elif defined(__arm__) || defined(__i386__)
#define SIZEOF_SIZE_T 4
#else
#error Unknown Architecture
#endif
#elif defined(_MSC_VER)
#if defined(__clang__)
 // CLANG-CL
#if defined(__aarch64__) || defined(__x86_64__)
#define SIZEOF_SIZE_T 8
#elif defined(__arm__) || defined(__i386__)
#define SIZEOF_SIZE_T 4
#else
#error Unknown Architecture
#endif
#else
 // MSVC
#if defined(_M_ARM64) || defined(_M_X64)
#define SIZEOF_SIZE_T 8
#elif defined(_M_ARM) || defined(_M_IX86)
#define SIZEOF_SIZE_T 4
#else
#error Unknown Architecture
#endif
#endif
#else
#error Unknown Compiler
#endif

/* Define to 1 if you have the <intrin.h> header file. */
#define HAVE_INTRIN_H

#if defined(_MSC_VER) && defined(HAVE_INTRIN_H)
#if (SIZEOF_SIZE_T == 8)
#define HAVE_BITSCANFORWARD64
#elif (SIZEOF_SIZE_T == 4)
#define HAVE_BITSCANFORWARD
#endif
#endif

#if defined(__has_attribute)
#if __has_attribute(fallthrough)
#define FALLTHROUGH  __attribute__((fallthrough));
#else
#define FALLTHROUGH
#endif
#else
#define FALLTHROUGH
#endif
