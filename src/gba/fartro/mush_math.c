#include "mush_math.h"

void math_fastdiv (s32 n, s32 d, s32 *result, s32 *remainder) {
	__asm__ volatile (
		"mov r0,%2 \n"
		"mov r1,%3 \n"
		"swi 0x60000 \n"
		"ldr r2,%0 \n"
		"str r0,[r2] \n"
		"ldr r2,%1 \n"
		"str r1,[r2] \n"
		: "=m" (result), "=m" (remainder)
		: "r" (n), "r" (d)
		: "r0" , "r1" , "r2" , "r3"
	);
}
