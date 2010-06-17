// types.h: typedefs
//
// (Created: 2003-05-05, Modified: 2004-01-06, Cearn)

#ifndef GBA_TYPES
#define GBA_TYPES

// === basic types: ===================================================
typedef unsigned char  u8,  byte;
typedef unsigned short u16, hword;
typedef unsigned int   u32, word;
typedef unsigned long long u64;

typedef signed char  s8;
typedef signed short s16; 
typedef signed int   s32;
typedef signed long long s64;

// and volatiles for registers 'n stuff
typedef volatile u8  vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8  vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef s32 FIXED;
typedef u16 COLOR;
typedef u16 TEGEL, MAPENTRY;
typedef u8 AFFTEGEL, AFFMAPENTRY;

// function pointer
typedef void (*fnptr)(void);

#ifndef NULL
#define NULL 0
#endif

#ifndef BOOL
typedef int BOOL;
#define TRUE 1
#define FALSE 0
#endif

// `inline' inlines the function when -O > 0 when called, 
// but also creates a body for the function itself
// `static' removes the body as well
#define INLINE static inline

#endif // GBA_TYPES

