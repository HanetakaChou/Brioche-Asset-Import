// Adapted from the autotools src/webp/config.h.in.

// Define if building universal (internal helper macro)
#undef AC_APPLE_UNIVERSAL_BUILD

// HAVE_BUILTIN_BSWAP16: Set to 1 if __builtin_bswap16 is available
// HAVE_BUILTIN_BSWAP32: Set to 1 if __builtin_bswap32 is available
// HAVE_BUILTIN_BSWAP64: Set to 1 if __builtin_bswap64 is available
#if defined(__GNUC__)
// GCC or CLANG
#define HAVE_BUILTIN_BSWAP16 1
#define HAVE_BUILTIN_BSWAP32 1
#define HAVE_BUILTIN_BSWAP64 1
#elif defined(_MSC_VER)
#if defined(__clang__)
// CLANG-CL
#define HAVE_BUILTIN_BSWAP16 1
#define HAVE_BUILTIN_BSWAP32 1
#define HAVE_BUILTIN_BSWAP64 1
#else
// MSVC
#undef HAVE_BUILTIN_BSWAP16
#undef HAVE_BUILTIN_BSWAP32
#undef HAVE_BUILTIN_BSWAP64
#endif
#else
#error Unknown Compiler
#endif

// Define to 1 if you have the <cpu-features.h> header file.
#undef HAVE_CPU_FEATURES_H

// Define to 1 if you have the <GLUT/glut.h> header file.
#undef HAVE_GLUT_GLUT_H

// Define to 1 if you have the <GL/glut.h> header file.
#undef HAVE_GL_GLUT_H

// Define to 1 if you have the <OpenGL/glut.h> header file.
#undef HAVE_OPENGL_GLUT_H

// Define to 1 if you have the <shlwapi.h> header file.
#undef HAVE_SHLWAPI_H

// Define to 1 if you have the <unistd.h> header file.
#undef HAVE_UNISTD_H

// Define to 1 if you have the <wincodec.h> header file.
#undef HAVE_WINCODEC_H

// Define to 1 if you have the <windows.h> header file.
#undef HAVE_WINDOWS_H

// Define to the sub-directory in which libtool stores uninstalled libraries.
#undef LT_OBJDIR

// Name of package
#define PACKAGE "WebP"

// Define to the address where bug reports for this package should be sent.
#define PACKAGE_BUGREPORT "https://bugs.chromium.org/p/webp"

// Define to the full name of this package.
#define PACKAGE_NAME "WebP"

// Define to the full name and version of this package.
#define PACKAGE_STRING "WebP 1.3.2"

// Define to the one symbol short name of this package.
#define PACKAGE_TARNAME "WebP"

// Define to the home page for this package.
#define PACKAGE_URL "https://developers.google.com/speed/webp"

// Define to the version of this package.
#define PACKAGE_VERSION "1.3.2"

/* Version number of package */
#define VERSION "1.3.2"

// Set to 1 if GIF library is installed
#undef WEBP_HAVE_GIF

// Set to 1 if OpenGL is supported
#undef WEBP_HAVE_GL

// Set to 1 if JPEG library is installed
#define WEBP_HAVE_JPEG 1

// Set to 1 if NEON is supported
#undef WEBP_HAVE_NEON

// Set to 1 if runtime detection of NEON is enabled
#undef WEBP_HAVE_NEON_RTCD

// Set to 1 if PNG library is installed
#define WEBP_HAVE_PNG 1

// Set to 1 if SDL library is installed
#undef WEBP_HAVE_SDL

// Set to 1 if SSE2 is supported
#define WEBP_HAVE_SSE2 1

// Set to 1 if SSE4.1 is supported
#define WEBP_HAVE_SSE41 1

// Set to 1 if TIFF library is installed
#undef WEBP_HAVE_TIFF

// Enable near lossless encoding
#define WEBP_NEAR_LOSSLESS 1

// Undefine this to disable thread support.
#define WEBP_USE_THREAD 1

// Define WORDS_BIGENDIAN to 1 if your processor stores words with the most significant byte first (like Motorola and SPARC, unlike Intel).
#if defined(__GNUC__)
// GCC or CLANG
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN_) && __ORDER_LITTLE_ENDIAN_ == __BYTE_ORDER__
#undef WORDS_BIGENDIAN
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __ORDER_BIG_ENDIAN__ == __BYTE_ORDER__
#define WORDS_BIGENDIAN 1
#else
#error Unknown Byte Order
#endif
#elif defined(_MSC_VER)
// MSVC or CLANG-CL
#undef WORDS_BIGENDIAN
#else
#error Unknown Compiler
#endif
