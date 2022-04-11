/*
	NOTE(Douglas): To be included for different OSs platform layers.
*/



/* NOTE: Detecting current compiler */
#if defined(_MSC_VER)
	#define MSVC_COMPILER (1)
	#define LLVM_COMPILER (0)
	#define GCC_COMPILER (0)
	#define INTEL_COMPILER (0)
#elif defined(__INTEL_COMPILER)
	#define MSVC_COMPILER (0)
	#define LLVM_COMPILER (0)
	#define GCC_COMPILER (0)
	#define INTEL_COMPILER (1)
#elif defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
	#define MSVC_COMPILER (0)
	#define LLVM_COMPILER (0)
	#define GCC_COMPILER (1)
	#define INTEL_COMPILER (0)
#elif defined(__llvm__)
	#define MSVC_COMPILER (0)
	#define LLVM_COMPILER (1)
	#define GCC_COMPILER (0)
	#define INTEL_COMPILER (0)
#endif

/* NOTE: C89 inline macro for different compilers */
#if !defined(__cplusplus) && (__STDC_VERSION__ < 199901L)
	#if defined(MSVC_COMPILER)
		#define inline __forceinline
	#elif #defined(LLVM_COMPILER)
		#define inline __attribute__((always_inline))
	#elif defined(GCC_COMPILER)
		#define inline __attribute__((always_inline))
	#elif defined(INTEL_COMPILER)
		#define inline __attribute__((forceinline))
	#endif
#endif

#if defined(DEBUG)
	#define assert(expression) if(!(expression)) { *(int *)0 = 0; }
#else
	#define assert(expression)
#endif

#define min(a, b) ((a) < (b)) ? (a) : (b)
#define max(a, b) ((a) > (b)) ? (a) : (b)

#define TRUE (1)
#define FALSE (0)

#define global_variable static
#define internal static

#include <stdint.h>
typedef int32_t b32;
typedef float f32;
typedef double f64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;



/*
 * Simulation specific
*/

typedef struct
{
	s32 width;
	s32 height;
	s32 bytes_per_pixel;
	s32 line_stride;
	void *memory;
} pixel_buffer;
