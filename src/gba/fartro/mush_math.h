#ifndef MUSH_MATH
#define MUSH_MATH

#include <tonc_gba.h>

void math_fastdiv (s32 n, s32 d, s32 *result, s32 *remainder);

inline int __divsi3(int A, int B) { 
	s32 result;
	s32 remainder;
	math_fastdiv(A, B, &result, &remainder);
	return result;
}

inline unsigned int __udivsi3(unsigned int A, unsigned int B) {
	s32 result;
	s32 remainder;
	math_fastdiv((s32)A, (s32)B, &result, &remainder);
	return (unsigned int)result;
}

inline int __modsi3(int A, int B) {
	s32 result;
	s32 remainder;
	math_fastdiv((s32)A, (s32)B, &result, &remainder);
	return (unsigned int)remainder;
}


#endif